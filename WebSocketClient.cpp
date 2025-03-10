#include "WebSocketClient.h"
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

// Aliases for Boost.Beast, Boost.Asio, and Nlohmann.Json namespaces
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio; 
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

// Constructor for WebSocketClient
WebSocketClient::WebSocketClient() : resolver(ioc), ws(ioc, ssl_ctx) {
    // Configure the SSL context to use default verify paths for certificate validation
    ssl_ctx.set_default_verify_paths();
}

// Destructor for WebSocketClient
WebSocketClient::~WebSocketClient() {
    if (ws.is_open()) {
        // Gracefully close the WebSocket connection if it's open
        ws.close(websocket::close_code::normal);
    }
}

// Connects to the WebSocket server using the provided host and port
void WebSocketClient::connect(const std::string& host, const std::string& port) {
    // Resolve the host and port to an endpoint
    auto const results = resolver.resolve(host, port);

    // Establish a connection to the resolved endpoint
    auto ep = net::connect(beast::get_lowest_layer(ws), results);

    // Set the SNI hostname for SSL (required by many servers)
    if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str())) {
        throw beast::system_error(
            beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()),
            "Failed to set SNI hostname"
        );
    }

    // Perform the SSL handshake for a secure connection
    ws.next_layer().handshake(ssl::stream_base::client);

    // Perform the WebSocket handshake with the server
    ws.handshake(host + ":" + port, "/ws/api/v2/");
    std::cout << "WebSocket connected to " << host << ":" << port << std::endl;
}

// Subscribes to a specific channel with the given token
void WebSocketClient::subscribe(const std::string& channel, const std::string& token) {
    // Create a subscription payload in JSON format
    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "private/subscribe"},
        {"params", {
            {"access_token", token},
            {"channels", {channel}}
        }}
    };

    // Send the subscription request to the server
    ws.write(net::buffer(payload.dump()));
    std::cout << "Subscribed to channel: " << channel << std::endl;
}

// Listens for incoming WebSocket messages and processes them
void WebSocketClient::listen() {
    try {
        while (true) {
            beast::flat_buffer buffer;
            ws.read(buffer);

            // Parse the received message as JSON
            auto data = beast::buffers_to_string(buffer.data());
            json response = json::parse(data);

            // Calculate propagation delay if a timestamp is present in the message
            if (response.contains("params") && response["params"].contains("data") && 
                response["params"]["data"].contains("timestamp")) { 
                auto server_time = response["params"]["data"]["timestamp"].get<long long>();
                auto client_time = duration_cast<milliseconds>(
                    system_clock::now().time_since_epoch()
                ).count();

                auto propagation_delay = client_time - server_time;
                std::cout << "Propagation delay: " << propagation_delay << " ms" << std::endl;
            }

            // Lock the mutex and display the received message
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << "Received update: " << response.dump(4) << std::endl;
        }
    } catch (const std::exception& e) {
        // Handle any errors that occur during message reading
        std::cerr << "Error during WebSocket read: " << e.what() << std::endl;
    }
}

// Closes the WebSocket connection gracefully
void WebSocketClient::close() {
    std::cout << "Closing WebSocket connection...\n";
    // Additional cleanup logic can be added here
}

// Starts a WebSocket session with a user-provided token
void startWebSocketSession(std::string& token) {
    WebSocketClient wsClient;

    // Connect to the WebSocket server
    wsClient.connect("test.deribit.com", "443");

    // Prompt the user to enter the instrument/symbol to subscribe to
    std::cout << "Enter the instrument/symbol (e.g., BTC-PERPETUAL) you want to subscribe:\n";
    std::string symbol;
    std::cin >> symbol;

    // Prompt the user to choose the update interval
    std::cout << "Choose the interval:\n1. 100ms\n2. raw\n3. agg2\n";
    int intervalChoice;
    std::cin >> intervalChoice;

    // Construct the subscription string based on user input
    std::string subscription = "book." + symbol;
    if (intervalChoice == 1)
        subscription += ".100ms";
    else if (intervalChoice == 2)
        subscription += ".raw";
    else
        subscription += ".agg2";

    // Subscribe to the specified channel
    wsClient.subscribe(subscription, token);

    // Start a thread to listen for incoming WebSocket messages
    std::thread listener([&wsClient]() { wsClient.listen(); });

    // Wait for the listener thread to finish (blocks the main thread)
    listener.join();
}
