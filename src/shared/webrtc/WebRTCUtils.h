// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_WEBRTC_UTILS_H_
#define MUMBLE_WEBRTC_UTILS_H_

#include <QString>
#include <QByteArray>
#include <string>

namespace Mumble {
namespace WebRTC {

class Utils {
public:
	static QString stdStringToQString(const std::string& str);
	static std::string qStringToStdString(const QString& str);
	static QByteArray rtpToMumbleAudio(const uint8_t* data, size_t len);
	static QByteArray mumbleAudioToRtp(const uint8_t* data, size_t len);
};

} // namespace WebRTC
} // namespace Mumble

#endif // MUMBLE_WEBRTC_UTILS_H_
