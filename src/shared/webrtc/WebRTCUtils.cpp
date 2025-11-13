// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "WebRTCUtils.h"

namespace Mumble {
namespace WebRTC {

QString Utils::stdStringToQString(const std::string& str) {
	return QString::fromStdString(str);
}

std::string Utils::qStringToStdString(const QString& str) {
	return str.toStdString();
}

QByteArray Utils::rtpToMumbleAudio(const uint8_t* data, size_t len) {
	// TODO: Implement RTP to Mumble audio conversion
	// For now, just copy the data directly
	return QByteArray(reinterpret_cast<const char*>(data), static_cast<int>(len));
}

QByteArray Utils::mumbleAudioToRtp(const uint8_t* data, size_t len) {
	// TODO: Implement Mumble audio to RTP conversion
	// For now, just copy the data directly
	return QByteArray(reinterpret_cast<const char*>(data), static_cast<int>(len));
}

} // namespace WebRTC
} // namespace Mumble
