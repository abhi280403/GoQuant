#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio; 
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();
    void connect(const std::string& host, const std::string& port);
    void subscribe(const std::string& subscription, const std::string& accessToken);
    void listen();
    void close();

private:
    net::io_context ioc;
    ssl::context ssl_ctx{ssl::context::tlsv12_client};
    tcp::resolver resolver;
    websocket::stream<beast::ssl_stream<tcp::socket>> ws;
    std::mutex mutex_;
};

void startWebSocketSession(std::string& token);

#endif
