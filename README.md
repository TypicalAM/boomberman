# Boomerman

A multiplayer Bomberman-style game called "Boomerman" using the [Raylib](https://www.raylib.com/) game development library for graphics and [Protocol Buffers](https://protobuf.dev/) for message interchange between game clients and the server. This repository also has a test client written in golang.

Components:
- Raylib: Handles game graphics, input, and rendering.
- Protocol Buffers: Defines message formats for communication between clients and the server.

High-level architecture:
- Game Server: Manages game logic, state, and client communication using Protocol Buffers.
- Game Clients: Communicate with the server for game updates and display state using Raylib.

Workflow:
- Clients connect to the server and send/receive messages using Protocol Buffers.
- Server processes messages, updates game state, and sends updates to clients.
- Clients render game state using Raylib based on received data.

Considerations:
- Networking: Appropriate use of sockets for client-server communication.
- Game Logic: Implement Bomberman-style game rules on the server side.
- Serialization: Use Protocol Buffers for efficient message serialization/deserialization. This is the choice because it enhances rapid iteration - we do not need to reimplement different encoders and decoders for message types.
