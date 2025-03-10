#include <iostream>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>
#include "APIClient.h"
#include "WebSocketClient.h"
#include "Utils.h"

using json = nlohmann::json;
using namespace std;

int main() {
    cout << "Starting Deribit Client..." << endl;
    WebSocketClient wsClient;
    DeribitClient client;
    // Authenticate and get token
    json token = client.getAuthToken();
    if (token.empty()) {
        cerr << "Authentication failed. Exiting...\n";
        return 1;
    }
    string accessToken = token;
    int ch;
    cout << "Establish connection via:" << endl;
    cout << "1. WebSocket Connection\n";
    cout << "2. HTTP connection\n";
    cout << "3. Exit from the app\n";
    cout << "Enter your choice: ";


    do{
    cin >> ch;
    switch(ch){
        case 1:{
            std::thread webSocketThread(startWebSocketSession, std::ref(accessToken));
            webSocketThread.join();
            break;
        }
        case 2:{
            int choice;
            do {
                displayMenu();
                cin >> choice;
                cin.ignore(); // To ignore any leftover input in the buffer

                switch (choice) {
                case 1: {
                    // Place Order
                    string instrument, type, label;
                    double amount, price = 0.0;
                    cout << "Enter instrument name: ";
                    cin >> instrument;
                    cout << "Enter order type (limit/market/stop_limit/...): ";
                    cin >> type;
                    cout << "Enter amount: ";
                    cin >> amount;
                    if (type == "limit" || type == "stop_limit") {
                        cout << "Enter price (Note that amount should be multiple of price): ";
                        cin >> price;
                    }
                    cout << "Enter label (optional): ";
                    cin.ignore();
                    getline(cin, label);
                    auto start_time = std::chrono::high_resolution_clock::now();
                    json response = client.placeOrder(accessToken, instrument, type, amount, price, label);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                    cout << "Response: " << response.dump(4) << endl;
                    cout << "Order placement latency: " << latency << " milliseconds" << endl;
                    break;
                }
                case 2: {
                    // Modify Order
                    string order_id;
                    double amount = 0, price = 0;
                    cout << "Enter order ID: ";
                    cin >> order_id;
                    cout << "Enter new amount: ";
                    cin >> amount;
                    cout << "Enter new price (or 0 for market order): ";
                    cin >> price;
                    json response = client.modifyOrder(order_id, accessToken, amount, nullopt, price);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 3: {
                    // Sell Order
                    string instrument, type;
                    double amount, price = 0.0;
                    cout << "Enter instrument name: ";
                    cin >> instrument;
                    cout << "Enter amount: ";
                    cin >> amount;
                    cout << "Enter order type (limit/market/stop_limit/...): ";
                    cin >> type;
                    if (type == "limit" || type == "stop_limit") {
                        cout << "Enter price (Note that amount should be multiple of price): ";
                        cin >> price;
                    }
                    json response = client.sellOrder(accessToken, instrument, amount, nullopt, price,type);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 4: {
                    // Cancel Order
                    string order_id;
                    cout << "Enter order ID: ";
                    cin >> order_id;
                    json response = client.cancelOrder(order_id, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 5: {
                    // Cancel All Orders
                    json response = client.cancelAllOrder(accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 6: {
                    // Get Open Orders
                    json response = client.getOpenOrder(accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 7: {
                    // Get Open Orders by Currency
                    string currency;
                    cout << "Enter currency (e.g., BTC): ";
                    cin >> currency;
                    json response = client.getOpenOrderByCurrency(currency, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 8: {
                    // Get Open Orders by Instrument
                    string instrument;
                    cout << "Enter instrument name: ";
                    cin >> instrument;
                    json response = client.getOpenOrderByInstrument(instrument, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 9: {
                    // Get Order State
                    string order_id;
                    cout << "Enter order ID: ";
                    cin >> order_id;
                    json response = client.getOrderState(order_id, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 10: {
                    // Get Order History by Currency
                    string currency;
                    cout << "Enter currency (e.g., BTC): ";
                    cin >> currency;
                    json response = client.getOrderHistoryByCurrency(currency, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 11: {
                    // Get Order History by Instrument
                    string instrument;
                    cout << "Enter instrument name: ";
                    cin >> instrument;
                    json response = client.getOrderHistoryByInstrument(instrument, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 12: {
                    // Get User Trades by Currency
                    string currency;
                    cout << "Enter currency (e.g., BTC): ";
                    cin >> currency;
                    json response = client.getUserTradesByCurrency(currency, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 13: {
                    // Get User Trades by Instrument
                    string instrument;
                    cout << "Enter instrument name: ";
                    cin >> instrument;
                    json response = client.getUserTradesByInstrument(instrument, accessToken);
                    cout << "Response: " << response.dump(4) << endl;
                    break;
                }
                case 14:
                    cout << "Exiting...\n";
                    break;
                default:
                    cout << "Invalid choice. Please try again.\n";
                }
            } while (choice != 14);
            break;
        }
        case 3:{
            cout << "Exiting...\n";
            break;
        }
        default:
            cout << "Invalid choice. Please try again.\n";
    }}while(ch!=3);
    return 0;
}
