#ifndef MOBLIDE_COMMUNICATION_H_
#define MOBLIDE_COMMUNICATION_H_

#include <pthread.h>

#include <iostream>

#include "cyber/component/component.h"
#include "cyber/cyber.h"
#include "modules/YOURMODULE/websocket_mobile/proto/mobile_config.pb.h"
#include "websocket_server.h"

using apollo::cyber::Component;
using apollo::cyber::Reader;
using apollo::cyber::Writer;
using YOURMODULE::websocket_mobile::Config;

class WebMoblie : public Component<> {
 public:
  bool Init() override;

 private:
  uint16_t port_;
  bool writter_init = false;
  std::string gps_topic;
  std::string mobile_pos_topic;
  // Websocketserver server_instance;
  // void SigHandle(int sig);
  bool LoadConfig(Config& m_config_);
  Config m_conf_;
};
CYBER_REGISTER_COMPONENT(WebMoblie)
#endif  // MOBLIDE_COMMUNICATION_H_
