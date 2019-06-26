/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2019 Centreon
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

#ifndef CCE_MACROS_DEFINES_HH_
# define CCE_MACROS_DEFINES_HH_

# include "com/centreon/engine/contact.hh"
# include "com/centreon/engine/contactgroup.hh"
# include "com/centreon/engine/host.hh"
# include "com/centreon/engine/hostgroup.hh"
# include "com/centreon/engine/service.hh"
# include "com/centreon/engine/servicegroup.hh"

// Length Limitations
# define MAX_COMMAND_ARGUMENTS                  32   // maximum number of $ARGx$ macros

// Macro Definitions
# define MACRO_ENV_VAR_PREFIX                   "NAGIOS_"
# define MAX_USER_MACROS                        256  // maximum number of $USERx$ macros

# define MACRO_HOSTNAME                         0
# define MACRO_HOSTALIAS                        1
# define MACRO_HOSTADDRESS                      2
# define MACRO_SERVICEDESC                      3
# define MACRO_SERVICESTATE                     4
# define MACRO_SERVICESTATEID                   5
# define MACRO_SERVICEATTEMPT                   6
# define MACRO_LONGDATETIME                     7
# define MACRO_SHORTDATETIME                    8
# define MACRO_DATE                             9
# define MACRO_TIME                             10
# define MACRO_TIMET                            11
# define MACRO_LASTHOSTCHECK                    12
# define MACRO_LASTSERVICECHECK                 13
# define MACRO_LASTHOSTSTATECHANGE              14
# define MACRO_LASTSERVICESTATECHANGE           15
# define MACRO_HOSTOUTPUT                       16
# define MACRO_SERVICEOUTPUT                    17
# define MACRO_HOSTPERFDATA                     18
# define MACRO_SERVICEPERFDATA                  19
# define MACRO_CONTACTNAME                      20
# define MACRO_CONTACTALIAS                     21
# define MACRO_CONTACTEMAIL                     22
# define MACRO_CONTACTPAGER                     23
# define MACRO_ADMINEMAIL                       24
# define MACRO_ADMINPAGER                       25
# define MACRO_HOSTSTATE                        26
# define MACRO_HOSTSTATEID                      27
# define MACRO_HOSTATTEMPT                      28
# define MACRO_NOTIFICATIONTYPE                 29
# define MACRO_NOTIFICATIONNUMBER               30  // deprecated - see HOSTNOTIFICATIONNUMBER and SERVICENOTIFICATIONNUMBER macros
# define MACRO_HOSTEXECUTIONTIME                31
# define MACRO_SERVICEEXECUTIONTIME             32
# define MACRO_HOSTLATENCY                      33
# define MACRO_SERVICELATENCY                   34
# define MACRO_HOSTDURATION                     35
# define MACRO_SERVICEDURATION                  36
# define MACRO_HOSTDURATIONSEC                  37
# define MACRO_SERVICEDURATIONSEC               38
# define MACRO_HOSTDOWNTIME                     39
# define MACRO_SERVICEDOWNTIME                  40
# define MACRO_HOSTSTATETYPE                    41
# define MACRO_SERVICESTATETYPE                 42
# define MACRO_HOSTPERCENTCHANGE                43
# define MACRO_SERVICEPERCENTCHANGE             44
# define MACRO_HOSTGROUPNAME                    45
# define MACRO_HOSTGROUPALIAS                   46
# define MACRO_SERVICEGROUPNAME                 47
# define MACRO_SERVICEGROUPALIAS                48
# define MACRO_HOSTACKAUTHOR                    49
# define MACRO_HOSTACKCOMMENT                   50
# define MACRO_SERVICEACKAUTHOR                 51
# define MACRO_SERVICEACKCOMMENT                52
# define MACRO_LASTSERVICEOK                    53
# define MACRO_LASTSERVICEWARNING               54
# define MACRO_LASTSERVICEUNKNOWN               55
# define MACRO_LASTSERVICECRITICAL              56
# define MACRO_LASTHOSTUP                       57
# define MACRO_LASTHOSTDOWN                     58
# define MACRO_LASTHOSTUNREACHABLE              59
# define MACRO_SERVICECHECKCOMMAND              60
# define MACRO_HOSTCHECKCOMMAND                 61
# define MACRO_MAINCONFIGFILE                   62
# define MACRO_STATUSDATAFILE                   63
# define MACRO_HOSTDISPLAYNAME                  64
# define MACRO_SERVICEDISPLAYNAME               65
# define MACRO_RETENTIONDATAFILE                66
# define MACRO_OBJECTCACHEFILE                  67
# define MACRO_TEMPFILE                         68
# define MACRO_LOGFILE                          69
# define MACRO_RESOURCEFILE                     70
# define MACRO_COMMANDFILE                      71
# define MACRO_HOSTPERFDATAFILE                 72
# define MACRO_SERVICEPERFDATAFILE              73
# define MACRO_HOSTACTIONURL                    74
# define MACRO_HOSTNOTESURL                     75
# define MACRO_HOSTNOTES                        76
# define MACRO_SERVICEACTIONURL                 77
# define MACRO_SERVICENOTESURL                  78
# define MACRO_SERVICENOTES                     79
# define MACRO_TOTALHOSTSUP                     80
# define MACRO_TOTALHOSTSDOWN                   81
# define MACRO_TOTALHOSTSUNREACHABLE            82
# define MACRO_TOTALHOSTSDOWNUNHANDLED          83
# define MACRO_TOTALHOSTSUNREACHABLEUNHANDLED   84
# define MACRO_TOTALHOSTPROBLEMS                85
# define MACRO_TOTALHOSTPROBLEMSUNHANDLED       86
# define MACRO_TOTALSERVICESOK                  87
# define MACRO_TOTALSERVICESWARNING             88
# define MACRO_TOTALSERVICESCRITICAL            89
# define MACRO_TOTALSERVICESUNKNOWN             90
# define MACRO_TOTALSERVICESWARNINGUNHANDLED    91
# define MACRO_TOTALSERVICESCRITICALUNHANDLED   92
# define MACRO_TOTALSERVICESUNKNOWNUNHANDLED    93
# define MACRO_TOTALSERVICEPROBLEMS             94
# define MACRO_TOTALSERVICEPROBLEMSUNHANDLED    95
# define MACRO_PROCESSSTARTTIME                 96
# define MACRO_HOSTCHECKTYPE                    97
# define MACRO_SERVICECHECKTYPE                 98
# define MACRO_LONGHOSTOUTPUT                   99
# define MACRO_LONGSERVICEOUTPUT                100
# define MACRO_TEMPPATH                         101
# define MACRO_HOSTNOTIFICATIONNUMBER           102
# define MACRO_SERVICENOTIFICATIONNUMBER        103
# define MACRO_HOSTNOTIFICATIONID               104
# define MACRO_SERVICENOTIFICATIONID            105
# define MACRO_HOSTEVENTID                      106
# define MACRO_LASTHOSTEVENTID                  107
# define MACRO_SERVICEEVENTID                   108
# define MACRO_LASTSERVICEEVENTID               109
# define MACRO_HOSTGROUPNAMES                   110
# define MACRO_SERVICEGROUPNAMES                111
# define MACRO_HOSTACKAUTHORNAME                112
# define MACRO_HOSTACKAUTHORALIAS               113
# define MACRO_SERVICEACKAUTHORNAME             114
# define MACRO_SERVICEACKAUTHORALIAS            115
# define MACRO_MAXHOSTATTEMPTS                  116
# define MACRO_MAXSERVICEATTEMPTS               117
# define MACRO_SERVICEISVOLATILE                118
# define MACRO_TOTALHOSTSERVICES                119
# define MACRO_TOTALHOSTSERVICESOK              120
# define MACRO_TOTALHOSTSERVICESWARNING         121
# define MACRO_TOTALHOSTSERVICESUNKNOWN         122
# define MACRO_TOTALHOSTSERVICESCRITICAL        123
# define MACRO_HOSTGROUPNOTES                   124
# define MACRO_HOSTGROUPNOTESURL                125
# define MACRO_HOSTGROUPACTIONURL               126
# define MACRO_SERVICEGROUPNOTES                127
# define MACRO_SERVICEGROUPNOTESURL             128
# define MACRO_SERVICEGROUPACTIONURL            129
# define MACRO_HOSTGROUPMEMBERS                 130
# define MACRO_SERVICEGROUPMEMBERS              131
# define MACRO_CONTACTGROUPNAME                 132
# define MACRO_CONTACTGROUPALIAS                133
# define MACRO_CONTACTGROUPMEMBERS              134
# define MACRO_CONTACTGROUPNAMES                135
# define MACRO_NOTIFICATIONRECIPIENTS           136
# define MACRO_NOTIFICATIONISESCALATED          137
# define MACRO_NOTIFICATIONAUTHOR               138
# define MACRO_NOTIFICATIONAUTHORNAME           139
# define MACRO_NOTIFICATIONAUTHORALIAS          140
# define MACRO_NOTIFICATIONCOMMENT              141
# define MACRO_EVENTSTARTTIME                   142
# define MACRO_HOSTPROBLEMID                    143
# define MACRO_LASTHOSTPROBLEMID                144
# define MACRO_SERVICEPROBLEMID                 145
# define MACRO_LASTSERVICEPROBLEMID             146
# define MACRO_ISVALIDTIME                      147
# define MACRO_NEXTVALIDTIME                    148
# define MACRO_LASTHOSTSTATE                    149
# define MACRO_LASTHOSTSTATEID                  150
# define MACRO_LASTSERVICESTATE                 151
# define MACRO_LASTSERVICESTATEID               152
# define MACRO_HOSTPARENTS                      153
# define MACRO_HOSTCHILDREN                     154
# define MACRO_HOSTID                           155
# define MACRO_SERVICEID                        156
# define MACRO_HOSTTIMEZONE                     157
# define MACRO_SERVICETIMEZONE                  158
# define MACRO_CONTACTTIMEZONE                  159
# define MACRO_X_COUNT                          160  // size of macro_x[] array

// Macro Cleaning Options
# define STRIP_ILLEGAL_MACRO_CHARS              1
# define ESCAPE_MACRO_CHARS                     2
# define URL_ENCODE_MACRO_CHARS                 4

// NAGIOS_MACROS structure
class nagios_macros {
 public:
  nagios_macros()
    : host_ptr{nullptr},
      hostgroup_ptr{nullptr},
      service_ptr{nullptr},
      servicegroup_ptr{nullptr},
      contact_ptr{nullptr},
      contactgroup_ptr{nullptr} {};

  std::array<std::string, MACRO_X_COUNT> x;
  std::array<std::string, MAX_COMMAND_ARGUMENTS> argv;
  std::array<std::string, MAX_CONTACT_ADDRESSES> contactaddress;
  std::string                           ondemand;
  com::centreon::engine::host*          host_ptr;
  com::centreon::engine::hostgroup*     hostgroup_ptr;
  com::centreon::engine::service*       service_ptr;
  com::centreon::engine::servicegroup*  servicegroup_ptr;
  com::centreon::engine::contact*       contact_ptr;
  com::centreon::engine::contactgroup*  contactgroup_ptr;
  std::unordered_map<std::string, com::centreon::engine::customvariable>
                                        custom_host_vars;
  std::unordered_map<std::string, com::centreon::engine::customvariable>
                                        custom_service_vars;
  std::unordered_map<std::string, com::centreon::engine::customvariable>
                                        custom_contact_vars;
};

#endif /* !CCE_MACROS_DEFINES_HH_ */
