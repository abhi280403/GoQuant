# Set CMake policies to suppress warnings
cmake_policy(SET CMP0144 NEW)  # Use upper-case <PACKAGENAME>_ROOT variables
cmake_policy(SET CMP0167 NEW)  # Handle the FindBoost module properly

# Set the minimum version of CMake required
cmake_minimum_required(VERSION 3.15)

# Set the project name
project(GoQuant)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find libcurl
find_package(CURL REQUIRED)

# Find Boost (only system component is needed)
find_package(Boost REQUIRED COMPONENTS system)

# Add your executable
add_executable(GoQuant main.cpp WebSocketClient.cpp APIClient.cpp)

# Link libraries to your executable
target_link_libraries(GoQuant PRIVATE 
    CURL::libcurl 
    Boost::system 
    OpenSSL::SSL 
    OpenSSL::Crypto
)