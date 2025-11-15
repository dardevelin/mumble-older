// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_WEBRTC_CONNECTION_H_
#define MUMBLE_MUMBLE_WEBRTC_CONNECTION_H_

#include <QObject>
#include <memory>

#ifdef USE_WEBRTC
#include <rtc/rtc.hpp>
#include "src/shared/webrtc/WebRTCConfig.h"

class WebRTCConnection : public QObject {
	Q_OBJECT
	
public:
	explicit WebRTCConnection(QObject *parent = nullptr);
	~WebRTCConnection();
	
	bool initialize(const Mumble::WebRTC::Configuration& config);
	void createOffer();
	void createAnswer(const QString &offerSdp);
	void setRemoteDescription(const QString &sdp);
	void addIceCandidate(const QString &candidate, const QString &sdpMid, int sdpMLineIndex);
	void close();
	
	bool isConnected() const { return m_connected; }
	
signals:
	void offerCreated(const QString &sdp);
	void answerCreated(const QString &sdp);
	void iceCandidateGenerated(const QString &candidate, const QString &sdpMid, int sdpMLineIndex);
	void connected();
	void disconnected();
	void audioDataReceived(const QByteArray &data);
	void error(const QString &message);
	
public slots:
	void sendAudioData(const QByteArray &data);
	
private:
	std::shared_ptr<rtc::PeerConnection> m_peerConnection;
	std::shared_ptr<rtc::Track> m_audioTrack;
	std::shared_ptr<rtc::DataChannel> m_dataChannel;
	Mumble::WebRTC::Configuration m_config;
	bool m_connected = false;
	
	void setupCallbacks();
	void handleLocalDescription(const rtc::Description &description);
	void handleLocalCandidate(const rtc::Candidate &candidate);
};

#endif // USE_WEBRTC
#endif // MUMBLE_MUMBLE_WEBRTC_CONNECTION_H_
