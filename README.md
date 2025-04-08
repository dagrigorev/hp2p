# High-Performance P2P Networking Stack

This project implements a high-performance Peer-to-Peer (P2P) networking stack using C++ that focuses on network efficiency, scalability, and optimized message exchange. The system is designed to use FlatBuffers for message serialization, IOCP (I/O Completion Ports) for networking, and provides a message-sharing model for communication between peers.

## Features

- **P2P Networking**: Establishes connections between peers for data exchange.
- **FlatBuffers**: Uses FlatBuffers for efficient message serialization and deserialization.
- **IOCP Networking**: Utilizes IOCP to manage asynchronous I/O operations and achieve high concurrency.
- **Message Sharing**: Peer-to-peer communication is based on message exchange.
- **Metric Monitoring**: Includes basic metrics to monitor network activity and provide warnings if thresholds are exceeded.

## Components

1. **Message Serialization**:
   - Uses FlatBuffers to serialize and deserialize messages efficiently.
   - Message types include `HELLO`, `PING`, `CHAT`, and `METRIC_WARNING`.

2. **Networking**:
   - Implements a basic P2P network model using IOCP for efficient, non-blocking communication.
   - Allows sending and receiving of messages across the network.

3. **Metrics and Warnings**:
   - Provides a metric monitoring system with thresholds for network performance.
   - Alerts (warnings) are triggered when performance metrics exceed the predefined limits.

## How to Build and Run

### Prerequisites

- **C++ Compiler**: Ensure you have a modern C++ compiler (e.g., Visual Studio 2022).
- **FlatBuffers**: Download and install [FlatBuffers](https://google.github.io/flatbuffers/).
- **CMake**: Used to generate project files for Visual Studio.

### Steps to Build

1. **Clone the Repository**:
    ```bash
    git clone https://github.com/your-repo/high-performance-p2p-network.git
    cd high-performance-p2p-network
    ```

2. **Install FlatBuffers**:
    - Download FlatBuffers from [here](https://google.github.io/flatbuffers/).
    - Follow the [installation guide](https://google.github.io/flatbuffers/quickstart.html).

3. **Generate FlatBuffers Code**:
    - Run the FlatBuffers compiler on the schema file (`message.fbs`) to generate C++ code:
      ```bash
      flatc --cpp message.fbs
      ```

4. **Build the Project**:
    - Use CMake to configure the project and generate Visual Studio project files:
      ```bash
      cmake -G "Visual Studio 2022" .
      ```
    - Open the generated `.sln` file in Visual Studio and build the project.

### Running the Project

1. **Create a Message**:
   - Run the program that serializes a message and writes it to a file (`message.bin`).

2. **Send the Message**:
   - Use `netcat` or any TCP client to send the `message.bin` over the network:
     ```bash
     nc <destination-ip> <port> < message.bin
     ```

3. **Receive the Message**:
   - The receiver will deserialize the message and print the contents:
     ```bash
     Sender ID: sender_123
     Message Type: CHAT
     Payload: Hello, this is a message
     ```

## Project Structure

```
high-performance-p2p-network/
│
├── CMakeLists.txt               # CMake build configuration
├── message.fbs                  # FlatBuffers schema
├── message_generated.h          # Generated message header from FlatBuffers schema
├── src/                         # Source files
│   ├── main.cpp                 # Entry point for the project
│   └── network.cpp              # Networking logic (IOCP)
├── build/                       # Build directory
└── README.md                    # Project README
```