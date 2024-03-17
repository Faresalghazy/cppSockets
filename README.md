# Socket Communication System PoC

## Overview

This project serves as a learning endeavor and a Proof of Concept (PoC) for a basic socket communication system. It is designed to facilitate a fundamental understanding of how socket programming works in C++ and to showcase simple client-server interaction. The system allows multiple clients to connect to a server, join a specific "room," and exchange messages within that room.

## Features

- **Server Setup**: A single server instance that listens for incoming client connections, handles joining/leaving rooms, and broadcasts messages to clients within the same room.
- **Pump Client**: A client designed to continuously send randomly generated "price feed" messages to simulate a data source.
- **Read Client**: A client that connects to the server to receive and display messages broadcasted within a specific room.

## Technical Details

- **Language**: The entire project is developed in C++.
- **Socket Library**: Uses Winsock2 for the Windows platform.
- **Concurrency**: Utilizes threads to handle multiple clients simultaneously.

## Project Structure

- `main.cpp` - Entry point for selecting and running the server, pump client, or read client.
- `SocketServer.h/cpp` - Implementation of the server's logic.
- `SocketClient.h/cpp` - Implementation of the client's logic.
- `README.md` - This file, providing an overview and instructions for the project.

## Usage

1. Compile the project using a C++ compiler that supports C++11 or later.
2. Run the executable. You will be prompted to select whether to start a server, pump client, or read client.
   - To start the server, enter `3`.
   - To start a pump client, enter `1`.
   - To start a read client, enter `2`.
3. Follow the on-screen instructions for each option.

https://github.com/Faresalghazy/cppSockets/assets/19968713/62e35422-ed79-4dcb-83ad-600d508a1831


## Disclaimer

This project is for educational purposes and serves as a basic demonstration of socket communication concepts. It is not intended for production use.

## Acknowledgments

This project was developed as part of a learning initiative to relearn the workings of network programming and socket communication in C++ in a weekend.

