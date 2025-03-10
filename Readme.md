# Deribit Trading API Client

This project provides a C++ client implementation for interacting with the Deribit cryptocurrency trading platform. It includes functionalities for managing orders, retrieving market data, and maintaining a WebSocket connection for real-time updates.

## Features

### REST API Functions
- **Order Management**:
  - Place buy orders.
  - Modify existing orders.
  - Place sell orders.
  - Cancel specific orders or all orders.
- **Order Information**:
  - Fetch open orders by currency or instrument.
  - Retrieve order state and history.
- **Market Data**:
  - Get instruments for a specific currency.
  - Access the order book for a specific trading symbol.
- **User Trades**:
  - Retrieve user trade history by currency or instrument.

### WebSocket API Functions
- Establishes a secure WebSocket connection with Deribit's server.
- Subscribes to real-time updates for specified instruments and intervals.
- Measures propagation delay between the server and client.
- Processes and displays live data streams.

## Project Structure
```
├── .vscode/               # VS Code configuration files
├── backup_code/           # Backup directory for previous versions of the code
├── build/                 # Directory for build artifacts
├── APIClient.cpp          # REST API client implementation
├── APIClient.h            # REST API client header file
├── CMakeLists.txt         # Build configuration file for CMake
├── main.cpp               # Entry point of the application
├── Readme.md              # Project documentation
├── tempCodeRunnerFile.cpp # Temporary file generated by the IDE
├── Utils.h                # Utility functions used in the project
├── WebSocketClient.cpp    # WebSocket client implementation
├── WebSocketClient.h      # WebSocket client header file
```

---

## Dependencies

### macOS Installation

1. **Homebrew**:
   - If you don't have Homebrew installed, install it first:
     ```bash
     /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
     ```

2. **cURL Library**:
   - Used for making HTTP requests to RESTful APIs.
   - **Installation**:
     ```bash
     brew install curl
     ```

3. **Boost Libraries**:
   - Provides robust support for networking, WebSocket communication, and SSL functionalities.
   - **Installation**:
     ```bash
     brew install boost
     ```

4. **OpenSSL Library**:
   - Ensures secure communication by providing TLS/SSL support.
   - **Installation**:
     ```bash
     brew install openssl
     ```

5. **nlohmann-json**:
   - A lightweight library for JSON serialization and deserialization.
   - **Installation**:
     ```bash
     brew install nlohmann-json
     ```

6. **CMake**:
   - Required for building the project.
   - **Installation**:
     ```bash
     brew install cmake
     ```

---

## Building the Project

1. **Set Up API Credentials**:
   - Open `APIClient.cpp` and replace `CLIENT_ID` and `CLIENT_SECRET` with your Deribit API credentials.

2. **Build the Project**:
   - Navigate to the project's root directory and run:
     ```bash
     mkdir -p build
     cd build
     cmake ..
     cmake --build .
     ```

3. **Run the Executable**:
   - After building, run the program:
     ```bash
     ./GoQuant
     ```

---

## Usage

1. **Authentication**:
   - The program will authenticate using your Deribit API credentials.

2. **Choose Connection Type**:
   - **WebSocket**: For real-time updates.
   - **HTTP**: For REST API interactions (e.g., placing orders, retrieving market data).

3. **Follow the Menu**:
   - Use the menu to perform actions like placing orders, modifying orders, or retrieving market data.

---

## Troubleshooting

### **1. Authentication Fails**
- Ensure `CLIENT_ID` and `CLIENT_SECRET` in `APIClient.cpp` are correct.
- Verify that your credentials are for the **Testnet** environment (since the code uses `test.deribit.com`).

### **2. Dependency Issues**
- If CMake cannot find a library, ensure it is installed via Homebrew.
- Manually specify library paths in `CMakeLists.txt` if needed:
  ```cmake
  set(BOOST_ROOT "/usr/local/opt/boost")
  set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
  ```

### **3. WebSocket Connection Issues**
- Ensure the WebSocket server URL (`test.deribit.com`) is correct and accessible.
- Check your internet connection and firewall settings.

---

## Author
- **Kancharla Abhijith Goud**
- B.Tech, Mnc, IIT Guwahati.

