#include "mobile_communication.h"
#pragma execution_character_set("utf-8")

bool flg_exit = false;
Websocketserver server_instance;
void SigHandle(int sig) {
  // server_instance.stop();
  server_instance.flg_exit = true;
  flg_exit = true;
}

bool WebMoblie::Init() {
  apollo::cyber::binary::SetName(node_->Name());
  if (!LoadConfig(m_conf_)) {
    return false;
  }
  // if (!gps_topic.empty() && !writter_init) {
  //   server_instance.InitWritter(gps_topic, mobile_pos_topic);
  //   writter_init = true;
  // }
  server_instance.m_gps_writer_ = node_->CreateWriter<Pose>(gps_topic);
  server_instance.m_mobile_msg_writer_ =
      node_->CreateWriter<MobilePos>(mobile_pos_topic);

  std::signal(SIGINT, SigHandle);

  try {
    // Websocketserver server_instance;
    // Start a thread to run the processing loop
    thread t(bind(&broadcast_server::process_messages, &server_instance));

    // Run the asio loop with the main thread
    if (server_instance.start(port_)) {
      std::cout << server_instance.geterrmsg() << std::endl;
    }
    t.join();
    if (flg_exit) {
      pthread_cancel(t.native_handle());
      return false;
    }
  } catch (websocketpp::exception const& e) {
    std::cout << e.what() << std::endl;
  }
  return true;
}

bool InnoWebMoblie::LoadConfig(Config& m_config_) {
  if (!GetProtoConfig(&m_config_)) {
    AERROR << "load config error";
    return false;
  }
  if (m_config_.has_port()) {
    port_ = m_config_.port();
  } else {
    AINFO << "Failed to load port. ";
    return false;
  }
  if (m_config_.has_mobile_pos_topic()) {
    mobile_pos_topic = m_config_.mobile_pos_topic();
  } else {
    AINFO << "Failed to load mobile_pos_topic name. ";
    return false;
  }

  return true;
}
