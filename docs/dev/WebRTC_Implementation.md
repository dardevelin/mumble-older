# WebRTC Implementation in Mumble

## Overview

This document describes the WebRTC integration in Mumble, which enables browser-based clients to connect to Mumble servers and communicate with native clients.

## Architecture

### Components

The WebRTC implementation consists of several key components:

1. **libdatachannel**: A lightweight WebRTC implementation library
2. **Protocol Extensions**: New protobuf messages for WebRTC signaling
3. **Client Classes**: WebRTC connection management in the client
4. **Server Classes**: WebRTC handler for server-side connections
5. **Shared Utilities**: Common configuration and utility functions

### Directory Structure

```
src/
├── Mumble.proto              # Extended with WebRTC messages
├── mumble/webrtc/            # Client-side WebRTC implementation
│   ├── WebRTCConnection.h    # Peer connection management
│   └── WebRTCConnection.cpp
├── murmur/webrtc/            # Server-side WebRTC implementation
│   ├── WebRTCHandler.h       # Server connection handler
│   └── WebRTCHandler.cpp
└── shared/webrtc/            # Shared WebRTC utilities
    ├── WebRTCConfig.h        # Configuration structures
    ├── WebRTCUtils.h         # Utility functions
    └── WebRTCUtils.cpp

3rdparty/libdatachannel/      # WebRTC implementation library
```

## Protocol Messages

### WebRTCSignal

Used for WebRTC signaling between client and server:

```protobuf
message WebRTCSignal {
    enum SignalType {
        OFFER = 0;
        ANSWER = 1;
        ICE_CANDIDATE = 2;
        DISCONNECT = 3;
    }
    
    optional SignalType type = 1;
    optional uint32 session = 2;
    optional string sdp = 3;
    optional string candidate = 4;
    optional string sdpMid = 5;
    optional uint32 sdpMLineIndex = 6;
}
```

### WebRTCCapabilities

Advertises server WebRTC support to clients:

```protobuf
message WebRTCCapabilities {
    optional bool supported = 1 [default = false];
    optional string signaling_url = 2;
    repeated string stun_servers = 3;
    repeated string turn_servers = 4;
}
```

## Client Implementation

### WebRTCConnection Class

The `WebRTCConnection` class manages WebRTC peer connections on the client side.

**Key Features:**
- Peer connection lifecycle management
- SDP offer/answer exchange
- ICE candidate gathering and exchange
- Data channel for audio transmission
- Qt signal/slot integration

**Usage Example:**

```cpp
#ifdef USE_WEBRTC
auto connection = new WebRTCConnection(this);

Mumble::WebRTC::Configuration config;
config.iceServers = {"stun:stun.l.google.com:19302"};
config.signalingUrl = "wss://server.example.com:8443";

if (connection->initialize(config)) {
    connect(connection, &WebRTCConnection::offerCreated,
            this, &MyClass::handleOffer);
    connection->createOffer();
}
#endif
```

## Server Implementation

### WebRTCHandler Class

The `WebRTCHandler` class manages WebRTC connections on the server side.

**Key Features:**
- Multiple peer connection management
- Per-user connection tracking
- SDP exchange handling
- ICE candidate relay
- Audio routing between WebRTC and native clients

**Usage Example:**

```cpp
#ifdef USE_WEBRTC
auto handler = new WebRTCHandler(server, this);
if (handler->initialize(8443)) {
    // Handle incoming offer from client
    handler->handleOffer(user, sdp);
}
#endif
```

## Building with WebRTC

### CMake Options

- `webrtc`: Enable WebRTC support (default: OFF)
- `webrtc-strict`: Require WebRTC support (default: OFF)

### Build Commands

```bash
# Clone the repository
git clone https://github.com/dardevelin/mumble-older.git
cd mumble-older

# Initialize submodules (including libdatachannel)
git submodule update --init --recursive

# Configure with WebRTC enabled
mkdir build
cd build
cmake .. -Dwebrtc=ON

# Build
make -j$(nproc)
```

### Dependencies

When building with WebRTC support, the following dependencies are required:

- All standard Mumble dependencies
- libdatachannel (included as submodule)
- CMake 3.23 or later

## WebRTC Data Flow

### Connection Establishment

1. Client initiates connection to server
2. Server sends `WebRTCCapabilities` message
3. Client creates `WebRTCConnection` and generates offer
4. Client sends `WebRTCSignal(OFFER)` to server
5. Server processes offer and generates answer
6. Server sends `WebRTCSignal(ANSWER)` to client
7. Both sides exchange ICE candidates via `WebRTCSignal(ICE_CANDIDATE)`
8. Connection established when ICE negotiation completes

### Audio Flow

1. Client captures audio via AudioInput
2. Audio encoded (Opus)
3. Audio sent via WebRTC data channel
4. Server receives audio via WebRTCHandler
5. Server routes audio to other clients (native and WebRTC)

## Configuration

### Client Configuration

WebRTC settings will be stored in the client's settings file:

```ini
[webrtc]
enabled=true
preferWebRTC=false
stunServers=stun:stun.l.google.com:19302
maxBitrate=64000
```

### Server Configuration

Server WebRTC options in `mumble-server.ini`:

```ini
; WebRTC Configuration
webrtcEnable=false
webrtcPort=8443
webrtcStunServers=stun:stun.l.google.com:19302
webrtcMaxBitrate=64000
```

## Security Considerations

1. **TLS/DTLS**: WebRTC connections use DTLS for encryption
2. **Authentication**: WebRTC clients must still authenticate via Mumble protocol
3. **STUN/TURN**: Proper STUN/TURN server configuration required for NAT traversal
4. **Signaling Security**: Signaling messages should be transmitted over encrypted connection

## Known Limitations

1. Audio conversion between RTP and Mumble format not yet fully implemented
2. WebSocket signaling server not yet implemented
3. No UI elements for WebRTC configuration
4. No automated tests for WebRTC functionality
5. Browser client implementation not included

## Future Work

1. **Signaling Server**: Implement WebSocket server for browser clients
2. **Audio Pipeline**: Complete audio format conversion
3. **Browser Client**: Create JavaScript/WebAssembly client
4. **Configuration UI**: Add WebRTC settings to client UI
5. **Testing**: Add comprehensive unit and integration tests
6. **Documentation**: Add user-facing documentation
7. **Performance**: Optimize audio pipeline and connection handling

## Troubleshooting

### Build Issues

**Problem**: CMake cannot find libdatachannel

**Solution**: Ensure submodules are initialized:
```bash
git submodule update --init --recursive
```

**Problem**: Compilation errors in WebRTC code

**Solution**: Ensure you're building with a compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)

### Runtime Issues

**Problem**: WebRTC connection fails

**Solution**: 
- Check that server has WebRTC enabled
- Verify STUN/TURN server configuration
- Check firewall rules for UDP ports

## Contributing

When contributing to the WebRTC implementation:

1. Ensure all code is guarded with `#ifdef USE_WEBRTC`
2. Add proper copyright headers
3. Follow Mumble coding style
4. Add tests for new functionality
5. Update documentation

## References

- [libdatachannel Documentation](https://github.com/paullouisageneau/libdatachannel)
- [WebRTC Specification](https://www.w3.org/TR/webrtc/)
- [Mumble Protocol Documentation](https://mumble-protocol.readthedocs.io/)
