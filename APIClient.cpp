#include "APIClient.h" // Include the header file for the API client
#include <iostream>    // For input/output operations
#include <optional>    // For using optional types
#include <string>      // For string operations
#include <set>         // For set data structure
#include <map>         // For map data structure
#include <curl/curl.h> // For handling HTTP requests via libcurl
#include <thread>      // For multithreading
#include <chrono>      // For time-related functions
#include <mutex>       // For thread-safe operations
#include <nlohmann/json.hpp> // For JSON handling using the nlohmann library

using json = nlohmann::json; // Alias for JSON type from nlohmann library
using namespace std; // Using the standard namespace
using namespace std::chrono; // For time utilities
using std::optional; // Optional type for handling optional values
using std::nullopt; // Null optional value

// Constants for API URL and credentials
const std::string API_URL = "https://test.deribit.com";
const std::string CLIENT_ID = "gZ8BxzqM"; // Replace with your client ID
const std::string CLIENT_SECRET = "83XncZMksNbjyOawCqk_4PUeWc_6IIHARk_ZLC712YI"; // Replace with your client secret

// Helper function for handling libcurl responses
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb); // Append response data to a string
    return size * nmemb; // Return the size of the processed data
}

// Constructor for the DeribitClient class
DeribitClient::DeribitClient() {
    curl_global_init(CURL_GLOBAL_ALL); // Initialize libcurl globally
    curl = curl_easy_init();          // Initialize a CURL easy session
}

// Destructor for the DeribitClient class
DeribitClient::~DeribitClient() {
    if (curl) curl_easy_cleanup(curl); // Clean up the CURL session
    curl_global_cleanup();             // Clean up global CURL resources
}

// URL encoding utility
std::string DeribitClient::urlEncode(const std::string& value) {
    CURL* curl = curl_easy_init();
    char* encoded = curl_easy_escape(curl, value.c_str(), value.length()); // Encode URL
    std::string encoded_str(encoded); // Convert to string
    curl_free(encoded);               // Free the encoded result
    curl_easy_cleanup(curl);          // Clean up CURL session
    return encoded_str;
}

// Function to send HTTP requests
json DeribitClient::sendRequest(const std::string& endpoint, const json& payload, const std::string& method, const std::string& token) {
    std::string response; // String to store the HTTP response
    std::string url = API_URL + endpoint; // Construct the full URL
    
    // Append query parameters for GET requests
    if (method == "GET" && !payload.empty()) {
        url += "?";
        for (auto it = payload.begin(); it != payload.end(); ++it) {
            if (it.value().is_string()) {
                url += it.key() + "=" + it.value().get<std::string>() + "&"; // Add key-value pair
            } else {
                url += it.key() + "=" + it.value().dump() + "&"; // Add non-string values
            }
        }
        url.pop_back(); // Remove trailing '&'
    }

    // Setup HTTP headers
    struct curl_slist* headers = nullptr;
    if (!token.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str()); // Add authorization header
    }
    headers = curl_slist_append(headers, "Content-Type: application/json"); // Add content-type header

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set URL
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // Set headers
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Set POST method
        std::string payload_str = payload.dump(); // Serialize JSON payload
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str()); // Add POST data
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); // Pass the response string

    CURLcode res = curl_easy_perform(curl); // Perform the request
    curl_slist_free_all(headers); // Free allocated headers

    if (res != CURLE_OK) { // Check for CURL errors
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return {};
    }

    return json::parse(response); // Parse and return the JSON response
}

// Authentication to get a token
json DeribitClient::getAuthToken() {
    json payload = {
        {"grant_type", "client_credentials"},
        {"client_id", CLIENT_ID},
        {"client_secret", CLIENT_SECRET}
    };
    json response = sendRequest("/api/v2/public/auth", payload, "GET");
    if (response.contains("result") && response["result"].contains("access_token")) {
        return response["result"]["access_token"];
    }

    std::cerr << "Failed to authenticate: " << response.dump(4) << std::endl;
    return "";
}

// Place an order
json DeribitClient::placeOrder(const std::string& token, const std::string& instrument, const std::string& type, double amount, double price, const std::string& label) {
    json payload = {
        {"instrument_name", instrument},
        {"type", type},
        {"amount", amount}
    };

    if (type == "limit") {
        payload["price"] = price;
    }
    if (!label.empty()) {
        payload["label"] = label;
    }
    cout << "Placing order in progress..." << endl;
    return sendRequest("/api/v2/private/buy", payload, "GET", token);
}

// Function to modify an existing order on the Deribit platform
json DeribitClient::modifyOrder(const string& order_id, 
                const string& token,
                const optional<double>& amount,  // Optional new amount for the order
                const optional<double>& contracts,  // Optional new contract value for the order
                const optional<double>& price,  // Optional new price for the order
                const optional<string>& advanced,  // Optional advanced parameters (e.g., "stop_market")
                const optional<bool>& post_only,  // Optional flag for post-only order
                const optional<bool>& reduce_only // Optional flag for reduce-only order
                ) {    
    // Validation to ensure 'amount' and 'contracts' match if both are provided
    if (amount && contracts && *amount != *contracts) {
        cerr << "Error: 'amount' and 'contracts' must match if both are provided." << endl;
        return {}; // Return an empty JSON object on validation failure
    }
    if (!amount && !contracts) {
        cerr << "Error: Either 'amount' or 'contracts' must be provided." << endl;
        return {}; // Return an empty JSON object if neither is provided
    }
    
    // Construct the payload with the required and optional fields
    json payload;
    payload["order_id"] = order_id; // Mandatory order ID
    if (amount) payload["amount"] = *amount; // Add amount if provided
    if (contracts) payload["contracts"] = *contracts; // Add contracts if provided
    if (price) payload["price"] = *price; // Add price if provided
    if (advanced) payload["advanced"] = *advanced; // Add advanced parameter if provided
    if (post_only) payload["post_only"] = *post_only; // Add post-only flag if provided
    if (reduce_only) payload["reduce_only"] = *reduce_only; // Add reduce-only flag if provided

    cout << "Modifying order in progress..." << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/edit", payload, "GET", token);
}

// Function to place a sell order on the Deribit platform
json DeribitClient::sellOrder(const string& token,
                const string& instrument,  // The instrument to trade (e.g., BTC-PERPETUAL)
                const optional<double>& amount,  // Optional amount to sell
                const optional<double>& contracts,  // Optional contract value
                const optional<double>& price,  // Optional price for the order
                const optional<string>& type,  // Optional order type (e.g., "limit", "market")
                const optional<string>& trigger,  // Optional trigger type (e.g., "stop_loss")
                const optional<double>& trigger_price // Optional trigger price
                ) {

    // Construct the payload with all provided parameters
    json payload;
    payload["instrument_name"] = instrument; // Mandatory instrument name
    if (amount) payload["amount"] = *amount; // Add amount if provided
    if (contracts) payload["contracts"] = *contracts; // Add contracts if provided
    if (price) payload["price"] = *price; // Add price if provided
    if (type) payload["type"] = *type; // Add type if provided
    if (trigger) payload["trigger"] = *trigger; // Add trigger type if provided
    if (trigger_price) payload["trigger_price"] = *trigger_price; // Add trigger price if provided

    cout << "Selling in progress..." << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/sell", payload, "GET", token);
}

// Function to cancel a specific order by its ID
json DeribitClient::cancelOrder(const std::string& orderid, const std::string& token) {
    cout << "Cancelling order in progress" << endl;
    json payload = {
        {"order_id", orderid}, // Include the order ID in the payload
    };

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/cancel", payload, "GET", token);
}

// Function to cancel all open orders for the authenticated user
json DeribitClient::cancelAllOrder(const std::string& token) {
    cout << "Cancelling All Order..." << endl;

    // Send the request with an empty payload to the appropriate API endpoint
    return sendRequest("/api/v2/private/cancel_all", {}, "GET", token);
}

// Function to get all open orders for the authenticated user
json DeribitClient::getOpenOrder(const std::string& token) {
    cout << "Get All Open Orders..." << endl;

    // Send the request with an empty payload to the appropriate API endpoint
    return sendRequest("/api/v2/private/get_open_orders", {}, "GET", token);
}

// Function to get open orders filtered by currency
json DeribitClient::getOpenOrderByCurrency(const string& currency, const std::string& token) {
    json payload = {{"currency", currency}}; // Include currency in the payload
    cout << "Get All Open Orders for " << currency << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/get_open_orders_by_currency", payload, "GET", token);
}

// Function to get open orders filtered by instrument name
json DeribitClient::getOpenOrderByInstrument(const string& instrument, const std::string& token) {
    json payload = {{"instrument_name", instrument}}; // Include instrument name in the payload
    cout << "Get All Open Orders for " << instrument << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/get_open_orders_by_instrument", payload, "GET", token);
}

// Function to get the state of a specific order by its ID
json DeribitClient::getOrderState(const std::string& orderid, const string& token) {
    // Construct the payload with the order ID
    json payload = {{"order_id", orderid}};
    
    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/get_order_state", payload, "GET", token);
}

// Function to retrieve the order history for a specific currency
json DeribitClient::getOrderHistoryByCurrency(const string& currency, const std::string& token) {
    // Construct the payload with the currency parameter
    json payload = {{"currency", currency}};
    cout << "Getting Order History for " << currency << "..." << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/get_order_history_by_currency", payload, "GET", token);
}

// Function to retrieve the order history for a specific instrument
json DeribitClient::getOrderHistoryByInstrument(const string& instrument, const std::string& token) {
    // Construct the payload with the instrument name
    json payload = {{"instrument_name", instrument}};
    cout << "Getting Order History for " << instrument << "..." << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/get_order_history_by_instrument", payload, "GET", token);
}

// Function to retrieve the user's trade history for a specific currency
json DeribitClient::getUserTradesByCurrency(const string& currency, const std::string& token) {
    // Construct the payload with the currency parameter
    json payload = {{"currency", currency}};
    cout << "Getting User Trades for " << currency << "..." << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/get_user_trades_by_currency", payload, "GET", token);
}

// Function to retrieve the user's trade history for a specific instrument
json DeribitClient::getUserTradesByInstrument(const string& instrument, const std::string& token) {
    // Construct the payload with the instrument name
    json payload = {{"instrument_name", instrument}};
    cout << "Getting User Trades for " << instrument << "..." << endl;

    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/private/get_user_trades_by_instrument", payload, "GET", token);
}

// Function to get the list of tradable instruments for a specific currency
json DeribitClient::getInstruments(const std::string& currency) {
    // Construct the payload with the currency parameter
    json payload = {{"currency", currency}};
    
    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/public/get_instruments", payload, "GET");
}

// Function to retrieve the order book for a specific instrument
json DeribitClient::getOrderBook(const std::string& symbol) {
    // Construct the payload with the instrument name
    json payload = {{"instrument_name", symbol}};
    
    // Send the request to the appropriate API endpoint and return the response
    return sendRequest("/api/v2/public/get_order_book", payload, "GET");
}


