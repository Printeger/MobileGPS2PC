#include "websocket_server.h"

Websocketserver::Websocketserver() : broadcast_server() {}

Websocketserver::~Websocketserver() {}

bool Websocketserver::start(uint16_t port) {
  try {
    // listen on specified port
    m_server.listen(port);

    // Start the server accept loop
    m_server.start_accept();

    // Start the ASIO io_service run loop

    m_server.run();

    return true;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    strcpy(errmsg, e.what());
  }
  return false;
}

void Websocketserver::stop() {
  try {
    m_server.stop();
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

void Websocketserver::process_messages() {
  while (1) {
    sleep_ms(100);
    while (m_actions.empty()) {
      sleep_ms(100);
      continue;
    }
    if (!m_actions.empty()) {
      unique_lock<mutex> lock(m_action_lock);
      action a = m_actions.front();
      m_actions.pop();
      lock.unlock();

      if (a.type == SUBSCRIBE) {
        lock_guard<mutex> guard(m_connection_lock);
        m_connections.insert(a.hdl);
        // 群发
        // int num = m_connections.size();
        con_list::iterator it;
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
          m_server.send(*it, "SOMEONE SOMEWHERE ONLINE. ",
                        websocketpp::frame::opcode::value::text);
        }

        std::cout << "CLIENT CONNECTED. " << std::endl;
      } else if (a.type == UNSUBSCRIBE) {
        lock_guard<mutex> guard(m_connection_lock);
        m_connections.erase(a.hdl);
        // 群发
        con_list::iterator it;
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
          m_server.send(*it, "SOMEONE SOMEWHERE OFFLINE. ",
                        websocketpp::frame::opcode::value::text);
        }

        std::cout << "CLIENT DISCONNECTED" << std::endl;
      } else if (a.type == MESSAGE) {
        lock_guard<mutex> guard(m_connection_lock);
        raw_msg = a.msg->get_payload();
        std::string str = a.msg->get_payload();
        std::cout << "CLIENT RECIVE: " << raw_msg << std::endl;

        PubGPS(raw_msg);
        // TODO===stop when recive Ctr + C===

        if (flg_exit) {
          stop();
          return;
        }

        if (!strcmp(str.c_str(), "bye")) {
          stop();
          return;
        }

        // if (str.find("/all", 0) != std::string::npos) {
        //   // 回复所有
        //   con_list::iterator it;
        //   for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        //     a.msg->set_payload("SOMEONE SEND: \"" + str.substr(4) + "\"了");
        //     m_server.send(*it, a.msg);
        //   }
        //   continue;
        // }
        // // 回复单个
        // a.msg->set_payload("RECIVE: \"" + str +
        //                    "\", flg_exit: " + std::to_string(flg_exit));
        // m_server.send(a.hdl, a.msg);

      } else {
        // undefined.
      }
    }
  }
}

char* Websocketserver::geterrmsg() { return errmsg; }

static void sleep_ms(unsigned int secs) {
  struct timeval tval;
  tval.tv_sec = secs / 1000;
  tval.tv_usec = (secs * 1000) % 1000000;
  select(0, NULL, NULL, NULL, &tval);
}

void Websocketserver::SigHandle(int sig) {
  flg_exit = true;
  stop();
  return;
}

void Websocketserver::LLA2UTM(const std::vector<double>& lla_,
                              std::vector<double>& utm_) {
  double lon_, lat_, alt_;
  lon_ = lla_[1] / 180 * M_PI;
  lat_ = lla_[2] / 180 * M_PI;
  alt_ = lla_[3];
  proj_->LLAToUTM(1, 1, &lon_, &lat_, &alt_);

  utm_[0] = lon_;
  utm_[1] = lat_;
  utm_[2] = alt_;
  utm_[3] = lla_[4];  // heading
  utm_[4] = lla_[5];  // speed
  utm_[5] = lla_[6];  // accuracy
}

void Websocketserver::PubGPS(std::string raw_msg) {
  std::string tmp;
  std::stringstream ss;
  std::vector<double> tmp_gps;
  std::vector<double> tmp_utm(6);

  ss.clear();
  ss.str("");
  ss.str(raw_msg);
  while (getline(ss, tmp, ',')) {
    if (tmp.empty()) {
      tmp_gps.emplace_back(-1.0);
    } else {
      tmp_gps.emplace_back(std::stod(tmp));
    }
  }
  double lon_0, lat_0;
  if (!proj_init) {
    lon_0 = tmp_gps[1];
    lat_0 = tmp_gps[2];
    proj_ = std::make_shared<PJTransformer>(lon_0, lat_0);
    proj_init = true;
  }
  uint64_t tmp_timestamp = uint64_t(tmp_gps[0] * 1e9);
  double tmp_stamp_sec = tmp_gps[0];

  LLA2UTM(tmp_gps, tmp_utm);

  MobilePos mobile_pos_;
  mobile_pos_.mutable_header()->set_lidar_timestamp(tmp_timestamp);
  mobile_pos_.mutable_header()->set_timestamp_sec(tmp_stamp_sec);
  mobile_pos_.set_lontitude(tmp_gps[1]);
  mobile_pos_.set_latitude(tmp_gps[2]);
  mobile_pos_.set_altitude(tmp_gps[3]);
  mobile_pos_.set_utm_x(tmp_utm[0]);
  mobile_pos_.set_utm_y(tmp_utm[1]);
  mobile_pos_.set_utm_z(tmp_utm[2]);
  mobile_pos_.set_heading(tmp_gps[4]);
  mobile_pos_.set_speed(tmp_gps[5]);
  mobile_pos_.set_accuracy(tmp_gps[6]);

  m_mobile_msg_writer_->Write(mobile_pos_);

  // std::cout << tmp_gps[0] << " " << tmp_gps[1] << " " << tmp_gps[2]
  //           << std::endl;
}
