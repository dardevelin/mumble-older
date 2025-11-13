// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_WEBRTC_CONFIG_H_
#define MUMBLE_WEBRTC_CONFIG_H_

#include <string>
#include <vector>

namespace Mumble {
namespace WebRTC {

struct Configuration {
	std::vector<std::string> iceServers;
	std::string signalingUrl;
	int audioSampleRate = 48000;
	int audioChannels = 1;
	bool useDataChannel = true;
	bool useReliableDataChannel = true;
	int maxBitrate = 64000; // 64kbps for Opus
};

} // namespace WebRTC
} // namespace Mumble

#endif // MUMBLE_WEBRTC_CONFIG_H_
