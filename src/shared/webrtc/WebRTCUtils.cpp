// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "WebRTCUtils.h"
#include <cstring>

namespace Mumble {
namespace WebRTC {

QString Utils::stdStringToQString(const std::string& str) {
	return QString::fromStdString(str);
}

std::string Utils::qStringToStdString(const QString& str) {
	return str.toStdString();
}

QByteArray Utils::rtpToMumbleAudio(const uint8_t* data, size_t len) {
	// RTP header is typically 12 bytes minimum
	// RTP Header format:
	// 0-1: Version (2 bits), Padding (1 bit), Extension (1 bit), CSRC count (4 bits), Marker (1 bit), Payload Type (7 bits)
	// 2-3: Sequence Number
	// 4-7: Timestamp
	// 8-11: SSRC
	// Then comes the payload (Opus audio data)
	
	if (len < 12) {
		// Not a valid RTP packet
		return QByteArray(reinterpret_cast<const char*>(data), static_cast<int>(len));
	}
	
	// Skip RTP header (12 bytes minimum)
	size_t headerSize = 12;
	
	// Check for CSRC identifiers
	uint8_t csrcCount = data[0] & 0x0F;
	headerSize += csrcCount * 4;
	
	// Check for extension header
	if ((data[0] & 0x10) != 0 && len > headerSize + 4) {
		// Extension present
		uint16_t extensionLength = (data[headerSize + 2] << 8) | data[headerSize + 3];
		headerSize += 4 + (extensionLength * 4);
	}
	
	if (headerSize >= len) {
		// Invalid packet
		return QByteArray();
	}
	
	// Extract the Opus payload (skip RTP header)
	const uint8_t* payload = data + headerSize;
	size_t payloadLen = len - headerSize;
	
	// Mumble expects raw Opus frames, so return the payload directly
	return QByteArray(reinterpret_cast<const char*>(payload), static_cast<int>(payloadLen));
}

QByteArray Utils::mumbleAudioToRtp(const uint8_t* data, size_t len) {
	// Create a minimal RTP header for Opus audio
	// We'll use a simplified RTP header:
	// - Version 2
	// - No padding, extension, or CSRC
	// - Payload type for Opus (typically 111 or dynamic)
	// - Sequence number and timestamp should be managed by the caller, using 0 for now
	
	QByteArray rtpPacket;
	rtpPacket.resize(12 + static_cast<int>(len)); // 12 byte header + payload
	
	uint8_t* header = reinterpret_cast<uint8_t*>(rtpPacket.data());
	
	// Byte 0: Version (2), Padding (0), Extension (0), CSRC count (0)
	header[0] = 0x80; // Version 2
	
	// Byte 1: Marker (0), Payload Type (111 for Opus)
	header[1] = 111; // Opus payload type
	
	// Bytes 2-3: Sequence number (should be incremented by caller, using 0 for now)
	header[2] = 0;
	header[3] = 0;
	
	// Bytes 4-7: Timestamp (should be set by caller based on sample rate, using 0 for now)
	header[4] = 0;
	header[5] = 0;
	header[6] = 0;
	header[7] = 0;
	
	// Bytes 8-11: SSRC (Synchronization source identifier, using 0 for now)
	header[8] = 0;
	header[9] = 0;
	header[10] = 0;
	header[11] = 0;
	
	// Copy the Opus payload
	std::memcpy(header + 12, data, len);
	
	return rtpPacket;
}

} // namespace WebRTC
} // namespace Mumble
