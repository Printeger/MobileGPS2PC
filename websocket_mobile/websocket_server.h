#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include <boost/thread/mutex.hpp>
#include <csignal>
#include <iostream>
#include <set>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "cyber/component/component.h"
#include "cyber/cyber.h"
#include "cyber/node/node.h"
#include "gps_proj.hpp"
#include "modules/omnisense/common/proto/pose.pb.h"
#include "modules/omnisense/websocket_mobile/proto/mobile_pos_msg.pb.h"

using apollo::cyber::Node;
using apollo::cyber::Reader;
using apollo::cyber::Writer;
using YOURMODULE::websocket_mobile::MobilePos;

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

using websocketpp::lib::condition_variable;
using websocketpp::lib::lock_guard;
using websocketpp::lib::mutex;
using websocketpp::lib::thread;
using websocketpp::lib::unique_lock;

/* on_open insert connection_hdl into channel
 * on_close remove connection_hdl from channel
 * on_message queue send to all channels
 */

enum action_type { SUBSCRIBE, UNSUBSCRIBE, MESSAGE };

struct action {
  action(action_type t, connection_hdl h) : type(t), hdl(h) {}
  action(action_type t, connection_hdl h, server::message_ptr m)
      : type(t), hdl(h), msg(m) {}

  action_type type;
  websocketpp::connection_hdl hdl;
  server::message_ptr msg;
};

static void sleep_ms(unsigned int secs);

class broadcast_server {
 public:
  broadcast_server() {
    // Initialize Asio Transport
    m_server.init_asio();

    // Register handler callbacks
    m_server.set_open_handler(bind(&broadcast_server::on_open, this, ::_1));
    m_server.set_close_handler(bind(&broadcast_server::on_close, this, ::_1));
    m_server.set_message_handler(
        bind(&broadcast_server::on_message, this, ::_1, ::_2));
  }
  virtual ~broadcast_server(){};

  void run(uint16_t port) {
    //  avoid colddown
    m_server.set_reuse_addr(true);
    // listen on specified port
    m_server.listen(port);

    // Start the server accept loop
    m_server.start_accept();

    // Start the ASIO io_service run loop
    try {
      m_server.run();
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }

  void on_open(connection_hdl hdl) {
    {
      lock_guard<mutex> guard(m_action_lock);
      // std::cout << "on_open" << std::endl;
      m_actions.push(action(SUBSCRIBE, hdl));
    }
    m_action_cond.notify_one();
  }

  void on_close(connection_hdl hdl) {
    {
      lock_guard<mutex> guard(m_action_lock);
      // std::cout << "on_close" << std::endl;
      m_actions.push(action(UNSUBSCRIBE, hdl));
    }
    m_action_cond.notify_one();
  }

  void on_message(connection_hdl hdl, server::message_ptr msg) {
    // queue message up for sending by processing thread
    {
      lock_guard<mutex> guard(m_action_lock);
      // std::cout << "on_message" << std::endl;
      m_actions.push(action(MESSAGE, hdl, msg));
    }
    m_action_cond.notify_one();
  }

  virtual void process_messages() {
    while (1) {
      unique_lock<mutex> lock(m_action_lock);

      while (m_actions.empty()) {
        m_action_cond.wait(lock);
      }

      action a = m_actions.front();
      m_actions.pop();

      lock.unlock();

      if (a.type == SUBSCRIBE) {
        lock_guard<mutex> guard(m_connection_lock);
        m_connections.insert(a.hdl);
      } else if (a.type == UNSUBSCRIBE) {
        lock_guard<mutex> guard(m_connection_lock);
        m_connections.erase(a.hdl);
      } else if (a.type == MESSAGE) {
        lock_guard<mutex> guard(m_connection_lock);

        con_list::iterator it;
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
          m_server.send(*it, a.msg);
        }
      } else {
        // undefined.
      }
    }
  }

 protected:
  typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

  server m_server;
  con_list m_connections;
  std::queue<action> m_actions;

  mutex m_action_lock;
  mutex m_connection_lock;
  condition_variable m_action_cond;
};

class Websocketserver : public broadcast_server {
 public:
  Websocketserver();
  ~Websocketserver();

 private:
  char errmsg[512];

 public:
  // void InitWritter(std::string& topic_, std::string& topic_2);
  bool start(uint16_t port);
  void stop();
  char* geterrmsg();
  void process_messages() override;
  void SigHandle(int sig);

  void PubGPS(std::string raw_msg);
  void LLA2UTM(const std::vector<double>& lla_, std::vector<double>& utm_);

  std::shared_ptr<Node> node_ = nullptr;
  std::shared_ptr<::apollo::cyber::Writer<Pose>> m_gps_writer_;
  std::shared_ptr<::apollo::cyber::Writer<MobilePos>> m_mobile_msg_writer_;

  std::shared_ptr<PJTransformer> proj_ = nullptr;
  bool proj_init = false;
  bool flg_exit = false;

  std::string raw_msg;
};
