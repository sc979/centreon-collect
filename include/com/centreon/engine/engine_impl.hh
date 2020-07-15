/*
 * Copyright 2020 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#ifndef CCE_ENGINERPC_ENGINE_IMPL_HH
#define CCE_ENGINERPC_ENGINE_IMPL_HH

#include "com/centreon/engine/namespace.hh"
#include "engine.grpc.pb.h"

CCE_BEGIN()
class engine_impl final : public Engine::Service {
  grpc::Status GetVersion(grpc::ServerContext* context,
                          const ::google::protobuf::Empty* /*request*/,
                          Version* response) override;
  grpc::Status GetStats(grpc::ServerContext* context,
                        const GenericString* request,
                        Stats* response) override;
  grpc::Status ProcessServiceCheckResult(grpc::ServerContext* context,
                                         const Check* request,
                                         CommandSuccess* response) override;
  grpc::Status ProcessHostCheckResult(grpc::ServerContext* context,
                                      const Check* request,
                                      CommandSuccess* response) override;
  grpc::Status NewThresholdsFile(grpc::ServerContext* context,
                                 const ThresholdsFile* request,
                                 CommandSuccess* response) override;
  grpc::Status GetHostsCount(grpc::ServerContext* context,
                             const ::google::protobuf::Empty*,
                             GenericValue*) override;
  grpc::Status GetContactsCount(grpc::ServerContext* context,
                                const ::google::protobuf::Empty*,
                                GenericValue*) override;
  grpc::Status GetServicesCount(grpc::ServerContext* context,
                                const ::google::protobuf::Empty*,
                                GenericValue*) override;
  grpc::Status GetServiceGroupsCount(grpc::ServerContext* context,
                                     const ::google::protobuf::Empty*,
                                     GenericValue*) override;
  grpc::Status GetContactGroupsCount(grpc::ServerContext* context,
                                     const ::google::protobuf::Empty*,
                                     GenericValue*) override;
  grpc::Status GetHostGroupsCount(grpc::ServerContext* context,
                                  const ::google::protobuf::Empty*,
                                  GenericValue*) override;
  grpc::Status GetServiceDependenciesCount(grpc::ServerContext* context,
                                           const ::google::protobuf::Empty*,
                                           GenericValue*) override;
  grpc::Status GetHostDependenciesCount(grpc::ServerContext* context,
                                        const ::google::protobuf::Empty*,
                                        GenericValue*) override;
  grpc::Status GetHost(grpc::ServerContext* context,
                       const HostIdentifier* request,
                       EngineHost* response) override;
  grpc::Status GetContact(grpc::ServerContext* context,
                          const ContactIdentifier* request,
                          EngineContact* response) override;
  grpc::Status GetService(grpc::ServerContext* context,
                          const ServiceIdentifier* request,
                          EngineService* response) override;
  grpc::Status AddHostComment(grpc::ServerContext* context,
                              const EngineComment* request,
                              CommandSuccess* response) override;
  grpc::Status AddServiceComment(grpc::ServerContext* context,
                                 const EngineComment* request,
                                 CommandSuccess* response) override;
  grpc::Status DeleteComment(grpc::ServerContext* context,
                             const GenericValue* request,
                             CommandSuccess* response) override;
  grpc::Status DeleteAllServiceComments(grpc::ServerContext* context,
                                        const ServiceIdentifier* request,
                                        CommandSuccess* response) override;
  grpc::Status DeleteAllHostComments(grpc::ServerContext* context,
                                     const HostIdentifier* request,
                                     CommandSuccess* response) override;
  grpc::Status RemoveHostAcknowledgement(grpc::ServerContext* context,
                                         const HostIdentifier* request,
                                         CommandSuccess* response) override;
  grpc::Status RemoveServiceAcknowledgement(grpc::ServerContext* context,
                                            const ServiceIdentifier* request,
                                            CommandSuccess* response) override;
  grpc::Status ScheduleHostDowntime(grpc::ServerContext* context,
                                    const ScheduleDowntimeIdentifier* request,
                                    CommandSuccess* response) override;
  grpc::Status ScheduleServiceDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleHostServicesDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleHostGroupHostsDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleHostGroupServicesDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleServiceGroupHostsDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleServiceGroupServicesDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleAndPropagateHostDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleAndPropagateTriggeredHostDowntime(
      grpc::ServerContext* context,
      const ScheduleDowntimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status ScheduleHostCheck(grpc::ServerContext* context,
                                 const HostCheckIdentifier* request,
                                 CommandSuccess* response) override;
  grpc::Status ScheduleHostServiceCheck(grpc::ServerContext* context,
                                        const HostCheckIdentifier* request,
                                        CommandSuccess* response) override;
  grpc::Status ScheduleServiceCheck(grpc::ServerContext* context,
                                    const ServiceCheckIdentifier* request,
                                    CommandSuccess* response) override;
  grpc::Status SignalProcess(grpc::ServerContext* context,
                             const EngineSignalProcess* request,
                             CommandSuccess* response) override;
  grpc::Status DeleteHostDowntime(grpc::ServerContext* context,
                                  const GenericValue* request,
                                  CommandSuccess* response) override;
  grpc::Status DeleteServiceDowntime(grpc::ServerContext* context,
                                     const GenericValue* request,
                                     CommandSuccess* response) override;
  grpc::Status DeleteHostDowntimeFull(grpc::ServerContext* context,
                                      const DowntimeCriterias* request,
                                      CommandSuccess* response) override;
  grpc::Status DeleteServiceDowntimeFull(grpc::ServerContext* context,
                                         const DowntimeCriterias* request,
                                         CommandSuccess* response) override;
  grpc::Status DeleteDowntimeByHostName(grpc::ServerContext* context,
                                        const DowntimeHostIdentifier* request,
                                        CommandSuccess* response) override;
  grpc::Status DeleteDowntimeByHostGroupName(
      grpc::ServerContext* context,
      const DowntimeHostGroupIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status DeleteDowntimeByStartTimeComment(
      grpc::ServerContext* context,
      const DowntimeStartTimeIdentifier* request,
      CommandSuccess* response) override;
  grpc::Status DelayHostNotification(grpc::ServerContext* context,
                                     const HostDelayIdentifier* request,
                                     CommandSuccess* response) override;
  grpc::Status DelayServiceNotification(grpc::ServerContext* context,
                                        const ServiceDelayIdentifier* request,
                                        CommandSuccess* response) override;
};

CCE_END()
#endif /* !CCE_ENGINERPC_ENGINE_IMPL_HH */
