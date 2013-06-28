/*
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/io/directory_entry.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::io;

parser::store parser::_store[] = {
  &parser::_store_into_map<command, &command::command_name>,
  &parser::_store_into_map<connector, &connector::connector_name>,
  &parser::_store_into_map<contact, &contact::contact_name>,
  &parser::_store_into_map<contactgroup, &contactgroup::contactgroup_name>,
  &parser::_store_into_map<host, &host::host_name>,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_map<hostgroup, &hostgroup::hostgroup_name>,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_map<servicegroup, &servicegroup::servicegroup_name>,
  &parser::_store_into_map<timeperiod, &timeperiod::timeperiod_name>
};

/**
 *  Default constructor.
 *
 *  @param[in] read_options Configuration file reading options
 *             (use to skip some object type).
 */
parser::parser(unsigned int read_options)
  : _config(NULL),
    _read_options(read_options) {

}

/**
 *  Destructor.
 */
parser::~parser() throw () {

}

/**
 *  Parse configuration file.
 *
 *  @param[in] path   The configuration file path.
 *  @param[in] config The state configuration to fill.
 */
void parser::parse(std::string const& path, state& config) {
  _config = &config;

  // parse the global configuration file.
  _parse_global_configuration(path);

  // parse configuration files.
  _apply(config.cfg_file(), &parser::_parse_object_definitions);
  // parse resource files.
  _apply(config.resource_file(), &parser::_parse_resource_file);
  // parse configuration directories.
  _apply(config.cfg_dir(), &parser::_parse_directory_configuration);

  // Apply template.
  _resolve_template();

  // Apply extended info.
  _apply_hostextinfo();
  _apply_serviceextinfo();

  // Fill state.
  // XXX XXX XXX
  _insert(_map_objects[object::command], config.commands());
  _insert(_map_objects[object::connector], config.connectors());
  _insert(_map_objects[object::contact], config.contacts());
  _insert(_map_objects[object::contactgroup], config.contactgroups());
  _insert(_lst_objects[object::hostdependency], config.hostdependencies());
  // _insert(_lst_objects[object::hostescalation], config.hostescalations());
  _insert(_map_objects[object::hostgroup], config.hostgroups());
  _insert(_map_objects[object::host], config.hosts());
  // _insert(_lst_objects[object::servicedependency], config.servicedependencies());
  // _insert(_lst_objects[object::serviceescalation], config.serviceescalations());
  _insert(_map_objects[object::servicegroup], config.servicegroups());
  _insert(_lst_objects[object::service], config.services());
  _insert(_map_objects[object::timeperiod], config.timeperiods());

  // cleanup.
  for (unsigned int i(0);
       i < sizeof(_lst_objects) / sizeof(_lst_objects[0]);
       ++i) {
    _lst_objects[i].clear();
    _map_objects[i].clear();
    _templates[i].clear();
  }
}

/**
 *  Add object into the list.
 *
 *  @param[in] obj The object to add into the list.
 */
void parser::_add_object(object_ptr obj) {
  if (obj->is_template())
    return;

  if (!obj->id())
    throw (engine_error() << "configuration: parse "
           << obj->type_name() << " failed: property missing in "
           "file '" << _current_path << "' on line " << _current_line);
  (this->*_store[obj->type()])(obj);
}

/**
 *  Add template into the list.
 *
 *  @param[in] obj The tempalte to add into the list.
 */
void parser::_add_template(object_ptr obj) {
  if (!obj->is_template())
    return;

  std::string const& name(obj->name());
  if (name.empty())
    throw (engine_error() << "configuration: parse "
           << obj->type_name() << " failed: property 'name' is "
           "missing in file '" << _current_path << "' on line "
           << _current_line);
  map_object& tmpl(_templates[obj->type()]);
  if (tmpl.find(name) != tmpl.end())
    throw (engine_error() << "configuration: parse "
           << obj->type_name() << " failed: " << name
           << " already exist");
  tmpl[name] = obj;
}

/**
 *  Apply parse method into list.
 *
 *  @param[in] lst   The list to apply action.
 *  @param[in] pfunc The method to apply.
 */
void parser::_apply(
       std::list<std::string> const& lst,
       void (parser::*pfunc)(std::string const&)) {
  for (std::list<std::string>::const_iterator
         it(lst.begin()), end(lst.end());
       it != end;
       ++it)
    (this->*pfunc)(*it);
}

/**
 *  Apply the host extended info.
 *
 *  @warning This function is for compatibility and has very
 *           poor performance. Didn't use extended info. If you
 *           want to use the generic template system.
 */
void parser::_apply_hostextinfo() {
  map_object& gl_hosts(_map_objects[object::host]);
  list_object const& hostextinfos(_lst_objects[object::hostextinfo]);
  for (list_object::const_iterator
         it(hostextinfos.begin()), end(hostextinfos.end());
       it != end;
       ++it) {
    // Get the current hostextinfo to check.
    hostextinfo_ptr obj(*it);

    list_host hosts;
    _get_objects_by_list_name(obj->hosts(), gl_hosts, hosts);
    _get_hosts_by_hostgroups_name(obj->hostgroups(), hosts);

    for (list_host::const_iterator it(hosts.begin()), end(hosts.end());
         it != end;
         ++it)
      (*it)->merge(*obj);
  }
}

/**
 *  Apply the service extended info.
 *
 *  @warning This function is for compatibility and has very
 *           poor performance. Didn't use extended info. If you
 *           want to use the generic template system.
 */
void parser::_apply_serviceextinfo() {
  map_object& gl_hosts(_map_objects[object::host]);
  list_object& gl_services(_lst_objects[object::service]);
  list_object const& serviceextinfos(_lst_objects[object::serviceextinfo]);
  for (list_object::const_iterator
         it(serviceextinfos.begin()), end(serviceextinfos.end());
       it != end;
       ++it) {
    // Get the current serviceextinfo to check.
    serviceextinfo_ptr obj(*it);

    list_host hosts;
    _get_objects_by_list_name(obj->hosts(), gl_hosts, hosts);
    _get_hosts_by_hostgroups_name(obj->hostgroups(), hosts);

    for (list_object::iterator
           it(gl_services.begin()), end(gl_services.end());
         it != end;
         ++it) {
      service_ptr svc(*it);
      if (svc->service_description() != obj->service_description())
        continue;

      list_host svc_hosts;
      _get_objects_by_list_name(svc->hosts(), gl_hosts, svc_hosts);
      _get_hosts_by_hostgroups_name(svc->hostgroups(), svc_hosts);

      bool found(false);
      for (list_host::const_iterator
             it_host(hosts.begin()), end(hosts.end());
           !found && it_host != end;
           ++it_host) {
        for (list_host::const_iterator
               it_svc_host(svc_hosts.begin()), end(svc_hosts.end());
             it_svc_host != end;
             ++it_svc_host) {
          if ((*it_host)->host_name() == (*it_svc_host)->host_name()) {
            svc->merge(*obj);
            found = true;
          }
        }
      }
    }
  }
}

/**
 *  Build the hosts list with hostgroups.
 *
 *  @param[in]     hostgroups The hostgroups.
 *  @param[in,out] hosts      The host list to fill.
 */
void parser::_get_hosts_by_hostgroups(
       hostgroup_ptr const& hostgroups,
       list_host& hosts) {
  _get_objects_by_list_name(hostgroups->members(), _map_objects[object::host], hosts);
  _get_hosts_by_hostgroups_name(hostgroups->hostgroup_members(), hosts);
}

/**
 *  Build the hosts list with list of hostgroups.
 *
 *  @param[in]     hostgroups The hostgroups list.
 *  @param[in,out] hosts      The host list to fill.
 */
void parser::_get_hosts_by_hostgroups_name(
       list_string const& lst_group,
       list_host& hosts) {
  map_object& gl_hostgroups(_map_objects[object::hostgroup]);
  for (list_string::const_iterator
         it(lst_group.begin()), end(lst_group.end());
       it != end;
       ++it) {
    map_object::iterator it_hostgroups(gl_hostgroups.find(*it));
    if (it_hostgroups != gl_hostgroups.end()) {
      hostgroup_ptr obj(it_hostgroups->second);
      _get_hosts_by_hostgroups(obj, hosts);
    }
  }
}

/**
 *  Build the object list with list of object name.
 *
 *  @param[in]     lst     The object name list.
 *  @param[in]     objects The object map to find object name.
 *  @param[in,out] out     The list to fill.
 */
template<typename T>
void parser::_get_objects_by_list_name(
       list_string const& lst,
       map_object& objects,
       std::list<shared_ptr<T> >& out) {
  for (list_string::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    map_object::iterator it_obj(objects.find(*it));
    if (it_obj != objects.end()) {
      shared_ptr<T> obj(it_obj->second);
      out.push_back(obj);
    }
  }
}

/**
 *  Compare two shared pointer object id.
 *
 *  @return True if the first object id is smaller than
 *          the second object id.
 */
template<typename T>
static bool smaller_id(shared_ptr<T> const& c1, shared_ptr<T> const& c2) {
  return (c1->id() < c2->id());
}

/**
 *  Insert objects into type T list and sort the new list by object id.
 *
 *  @param[in]  from The objects source.
 *  @param[out] to   The objects destination.
 */
template<typename T>
void parser::_insert(
       list_object const& from,
       std::set<shared_ptr<T> >& to) {
  for (list_object::const_iterator it(from.begin()), end(from.end());
       it != end;
       ++it)
    to.insert(*it);
  return ;
}

/**
 *  Insert objects into type T list and sort the new list by object id.
 *
 *  @param[in]  from The objects source.
 *  @param[out] to   The objects destination.
 */
template<typename T>
void parser::_insert(
       map_object const& from,
       std::set<shared_ptr<T> >& to) {
  for (map_object::const_iterator it(from.begin()), end(from.end());
       it != end;
       ++it)
    to.insert(it->second);
  return ;
}

/**
 *  Get the map object type name.
 *
 *  @param[in] objects  The map object.
 *
 *  @return The type name.
 */
std::string const& parser::_map_object_type(
                     map_object const& objects) const throw () {
  static std::string const empty("");
  map_object::const_iterator it(objects.begin());
  if (it == objects.end())
    return (empty);
  return (it->second->type_name());
}

/**
 *  Parse the directory configuration.
 *
 *  @param[in] path The directory path.
 */
void parser::_parse_directory_configuration(std::string const& path) {
  directory_entry dir(path);
  std::list<file_entry> const& lst(dir.entry_list("*.cfg"));
  for (std::list<file_entry>::const_iterator
         it(lst.begin()), end(lst.end());
       it != end;
       ++it)
    _parse_object_definitions(it->path());
}

/**
 *  Parse the global configuration file.
 *
 *  @param[in] path The configuration path.
 */
void parser::_parse_global_configuration(std::string const& path) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse global "
           "configuration failed: can't open file '" << path << "'");

  _config->cfg_main(path);

  _current_line = 0;
  _current_path = path;

  std::string input;
  while (misc::get_next_line(stream, input, _current_line)) {
    std::string key;
    std::string value;
    if (!misc::split(input, key, value, '=')
        || !_config->set(key, value))
      throw (engine_error() << "configuration: parse global "
             "configuration failed: invalid line "
             "'" << input << "' in file '" << path << "' "
             "on line " << _current_line);
  }
}

/**
 *  Parse the object definition file.
 *
 *  @param[in] path The object definitions path.
 */
void parser::_parse_object_definitions(std::string const& path) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse object "
           "definitions failed: can't open file '" << path << "'");

  _current_line = 0;
  _current_path = path;

  bool parse_object(false);
  object_ptr obj;
  std::string input;
  while (misc::get_next_line(stream, input, _current_line)) {
    // Check if is a valid object.
    if (obj.is_null()) {
      if (input.find("define") || !std::isspace(input[6]))
        throw (engine_error() << "configuration: parse object "
               "definitions failed: unexpected start definition in "
               "file '" << _current_path << "' on line "
               << _current_line);
      misc::trim_left(input.erase(0, 6));
      std::size_t last(input.size() - 1);
      if (input.empty() || input[last] != '{')
        throw (engine_error() << "configuration: parse object "
               "definitions failed: unexpected start definition in "
               "file '" << _current_path << "' on line "
               << _current_line);
      std::string const& type(misc::trim_right(input.erase(last)));
      obj = object::create(type);
      if (obj.is_null())
        throw (engine_error() << "configuration: parse object "
               "definitions failed: unknown object type name "
               "'" << type << "' in file '" << _current_path
               << "' on line " << _current_line);
      parse_object = (_read_options & (1 << obj->type()));
    }
    // Check if is the not the end of the current object.
    else if (input != "}") {
      if (parse_object) {
        if (!obj->parse(input))
          throw (engine_error() << "configuration: parse object "
                 "definitions failed: invalid line "
                 "'" << input << "' in file '" << _current_path
                 << "' on line " << _current_line);
      }
    }
    // End of the current object.
    else {
      if (parse_object) {
        if (obj->is_template())
          _add_template(obj);
        else
          _add_object(obj);
      }
      obj.clear();
    }
  }
}

/**
 *  Parse the resource file.
 *
 *  @param[in] path The resource file path.
 */
void parser::_parse_resource_file(std::string const& path) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse resources "
           "configuration failed: can't open file '" << path << "'");

  _current_line = 0;
  _current_path = path;

  std::string input;
  while (misc::get_next_line(stream, input, _current_line)) {
    try {
      std::string key;
      std::string value;
      if (!misc::split(input, key, value, '='))
        throw (engine_error() << "configuration: parse resources "
               "configuration failed: invalid line "
               "'" << input << "' in file '" << _current_path << "' "
               "on line " << _current_line);
      _config->user(key, value);
    }
    catch (std::exception const& e) {
      (void)e;
      throw (engine_error() << "configuration: parse resources "
             "configuration failed: invalid line "
             "'" << input << "' in file '" << _current_path << "' "
             "on line " << _current_line);
    }
  }
}

/**
 *  Resolve template for register objects.
 */
void parser::_resolve_template() {
  for (unsigned int i(0);
       i < sizeof(_templates) / sizeof(_templates[0]);
       ++i) {
    map_object& templates(_templates[i]);
    for (map_object::iterator
           it(_templates[i].begin()), end(_templates[i].end());
         it != end;
         ++it)
      it->second->resolve_template(templates);
  }

  for (unsigned int i(0);
       i < sizeof(_lst_objects) / sizeof(_lst_objects[0]);
       ++i) {
    map_object& templates(_templates[i]);
    for (list_object::iterator
           it(_lst_objects[i].begin()), end(_lst_objects[i].end());
         it != end;
         ++it)
      (*it)->resolve_template(templates);
  }

  for (unsigned int i(0);
       i < sizeof(_map_objects) / sizeof(_map_objects[0]);
       ++i) {
    map_object& templates(_templates[i]);
    for (map_object::iterator
           it(_map_objects[i].begin()), end(_map_objects[i].end());
         it != end;
         ++it)
      it->second->resolve_template(templates);
  }
}

/**
 *  Store object into the list.
 *
 *  @param[in] obj The object to store.
 */
void parser::_store_into_list(object_ptr obj) {
  _lst_objects[obj->type()].push_back(obj);
}

/**
 *  Store object into the map.
 *
 *  @param[in] obj The object to store.
 */
template<typename T, std::string const& (T::*ptr)() const throw ()>
void parser::_store_into_map(object_ptr obj) {
  shared_ptr<T> real(obj);
  map_object::iterator
    it(_map_objects[obj->type()].find((real.get()->*ptr)()));
  if (it != _map_objects[obj->type()].end())
    throw (engine_error() << "configuration: parse "
           << obj->type_name() << " failed: " << obj->name()
           << " already exist");
  _map_objects[obj->type()][(real.get()->*ptr)()] = real;
}
