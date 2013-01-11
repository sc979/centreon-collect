/*
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Perl Connector.
**
** Centreon Perl Connector is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Perl Connector is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Perl Connector. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCCP_EMBEDDED_PERL_HH
#  define CCCP_EMBEDDED_PERL_HH

#  include "com/centreon/connector/perl/namespace.hh"
#  include "com/centreon/unordered_hash.hh"
#  include <string>
#  include <sys/types.h>
#  include <EXTERN.h>
#  include <perl.h>

// Global Perl interpreter.
extern PerlInterpreter*    my_perl;

CCCP_BEGIN()

/**
 *  @class embedded_perl embedded_perl.hh "com/centreon/connector/perl/embedded_perl.hh"
 *  @brief Embedded Perl interpreter.
 *
 *  Embedded Perl interpreter wrapped in a singleton.
 */
class                      embedded_perl {
public:
                           ~embedded_perl();
  static embedded_perl&    instance();
  static void              load(int* argc, char*** argv, char*** env);
  pid_t                    run(std::string const& cmd, int fds[3]);
  static void              unload();

private:
                           embedded_perl(
                             int* argc,
                             char*** argv,
                             char*** env);
                           embedded_perl(embedded_perl const& ep);
  embedded_perl&           operator=(embedded_perl const& ep);

  umap<std::string, SV*>   _parsed;
  static char const* const _script;
  pid_t                    _self;
};

CCCP_END()

#endif // !CCCP_EMBEDDED_PERL_HH
