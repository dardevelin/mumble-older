# WebRTC Quick Start Guide

This guide will help you quickly get started with the WebRTC implementation in Mumble.

## Prerequisites

- Git
- CMake 3.23 or later
- C++20 compatible compiler
- All standard Mumble dependencies

## Quick Build

```bash
# 1. Clone the repository
git clone https://github.com/dardevelin/mumble-older.git
cd mumble-older

# 2. Initialize all submodules (including libdatachannel)
git submodule update --init --recursive

# 3. Create build directory
mkdir build && cd build

# 4. Configure with WebRTC enabled
cmake .. -Dwebrtc=ON -DCMAKE_BUILD_TYPE=Release

# 5. Build (use -j for parallel compilation)
make -j$(nproc)

# 6. Run the client
./mumble

# 7. Run the server
./mumble-server
```

## Verifying WebRTC Support

### Check if WebRTC is Compiled In

```bash
# Check client binary for WebRTC symbols
nm mumble | grep -i webrtc

# Check server binary for WebRTC symbols
nm mumble-server | grep -i webrtc
```

If you see output, WebRTC support is compiled in.

## Basic Usage

### Client-Side

The WebRTC implementation is automatically available when compiled with `-Dwebrtc=ON`. 

**Key Classes:**
- `WebRTCConnection`: Manages WebRTC peer connections
- Located in: `src/mumble/webrtc/WebRTCConnection.h`

### Server-Side

**Key Classes:**
- `WebRTCHandler`: Manages server-side WebRTC connections
- Located in: `src/murmur/webrtc/WebRTCHandler.h`

## Configuration Options

### CMake Options

| Option | Description | Default |
|--------|-------------|---------|
| `webrtc` | Enable WebRTC support | OFF |
| `webrtc-strict` | Require WebRTC support | OFF |

### Examples

```bash
# Build with WebRTC enabled
cmake .. -Dwebrtc=ON

# Build with WebRTC required (fail if not available)
cmake .. -Dwebrtc=ON -Dwebrtc-strict=ON

# Build without WebRTC (default)
cmake ..
```

## Testing Your Build

### Test 1: Verify Compilation

```bash
cd build
make -j$(nproc)

# Check for any compilation errors
echo $?  # Should output 0 for success
```

### Test 2: Check Binary Size

```bash
# Compare binary sizes with and without WebRTC

# With WebRTC
ls -lh mumble mumble-server

# The binaries should be slightly larger with WebRTC enabled
```

### Test 3: Runtime Check

```bash
# Run the client in debug mode
./mumble --version

# Run the server in debug mode
./mumble-server --version
```

## Common Issues

### Issue: Submodule not initialized

**Symptom**: CMake error about missing libdatachannel

**Solution**:
```bash
git submodule update --init --recursive
```

### Issue: Compilation errors in WebRTC code

**Symptom**: Errors in `WebRTCConnection.cpp` or `WebRTCHandler.cpp`

**Solution**: Ensure you have a C++20 compatible compiler:
```bash
# Check compiler version
g++ --version  # Should be 8.0 or later
clang++ --version  # Should be 7.0 or later
```

### Issue: Linking errors with datachannel

**Symptom**: Undefined reference to libdatachannel symbols

**Solution**: Clean and rebuild:
```bash
cd build
rm -rf *
cmake .. -Dwebrtc=ON
make -j$(nproc)
```

## File Locations

### Source Files

```
src/mumble/webrtc/
├── WebRTCConnection.h       # Client WebRTC connection class
└── WebRTCConnection.cpp

src/murmur/webrtc/
├── WebRTCHandler.h          # Server WebRTC handler class
└── WebRTCHandler.cpp

src/shared/webrtc/
├── WebRTCConfig.h           # Shared configuration
├── WebRTCUtils.h            # Shared utilities
└── WebRTCUtils.cpp
```

### Protocol Files

```
src/Mumble.proto             # Extended with WebRTC messages
```

### Build Configuration

```
CMakeLists.txt               # Root CMake with webrtc option
src/mumble/CMakeLists.txt    # Client CMake with WebRTC sources
src/murmur/CMakeLists.txt    # Server CMake with WebRTC sources
```

## Next Steps

1. **Read the full documentation**: See `docs/dev/WebRTC_Implementation.md`
2. **Understand the protocol**: See WebRTC messages in `src/Mumble.proto`
3. **Explore the code**: Start with `src/mumble/webrtc/WebRTCConnection.h`
4. **Run tests**: (TODO: Tests not yet implemented)

## Getting Help

- Check the main documentation: `docs/dev/WebRTC_Implementation.md`
- Review the implementation status: Check the PR description
- Look at example code in the WebRTC classes

## Contributing

If you want to contribute to the WebRTC implementation:

1. Fork the repository
2. Create a feature branch
3. Make your changes (ensure they follow Mumble's coding style)
4. Add tests (when test infrastructure is available)
5. Submit a pull request

## Current Limitations

- Audio pipeline not fully implemented
- No WebSocket signaling server yet
- No UI elements for WebRTC configuration
- No browser client yet

These will be addressed in future updates.
