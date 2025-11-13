// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MURMUR_WEBRTC_HANDLER_H_
#define MUMBLE_MURMUR_WEBRTC_HANDLER_H_

#include <QObject>
#include <QHash>
#include <memory>

#ifdef USE_WEBRTC
#include <rtc/rtc.hpp>

class Server;
class ServerUser;

class WebRTCHandler : public QObject {
	Q_OBJECT
	
public:
	explicit WebRTCHandler(Server *server, QObject *parent = nullptr);
	~WebRTCHandler();
	
	bool initialize(int port = 8443);
	void handleOffer(ServerUser *user, const QString &sdp);
	void handleAnswer(ServerUser *user, const QString &sdp);
	void handleIceCandidate(ServerUser *user, const QString &candidate, const QString &sdpMid, int sdpMLineIndex);
	void disconnectUser(ServerUser *user);
	
private:
	Server *m_server;
	QHash<unsigned int, std::shared_ptr<rtc::PeerConnection>> m_connections;
	
	void setupPeerConnection(ServerUser *user);
	void routeAudioToNative(ServerUser *user, const QByteArray &data);
	void routeAudioToWebRTC(ServerUser *user, const QByteArray &data);
};

#endif // USE_WEBRTC
#endif // MUMBLE_MURMUR_WEBRTC_HANDLER_H_
