// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "WebRTCConnection.h"

#ifdef USE_WEBRTC

#include "src/shared/webrtc/WebRTCUtils.h"

WebRTCConnection::WebRTCConnection(QObject *parent)
	: QObject(parent)
	, m_connected(false) {
}

WebRTCConnection::~WebRTCConnection() {
	close();
}

bool WebRTCConnection::initialize(const Mumble::WebRTC::Configuration& config) {
	m_config = config;
	
	try {
		rtc::Configuration rtcConfig;
		for (const auto& server : config.iceServers) {
			rtcConfig.iceServers.push_back(server);
		}
		
		m_peerConnection = std::make_shared<rtc::PeerConnection>(rtcConfig);
		setupCallbacks();
		
		return true;
	} catch (const std::exception& e) {
		emit error(QString("Failed to initialize WebRTC: %1").arg(e.what()));
		return false;
	}
}

void WebRTCConnection::setupCallbacks() {
	if (!m_peerConnection) {
		return;
	}
	
	m_peerConnection->onLocalDescription([this](rtc::Description description) {
		handleLocalDescription(description);
	});
	
	m_peerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
		handleLocalCandidate(candidate);
	});
	
	m_peerConnection->onStateChange([this](rtc::PeerConnection::State state) {
		if (state == rtc::PeerConnection::State::Connected) {
			m_connected = true;
			emit connected();
		} else if (state == rtc::PeerConnection::State::Disconnected ||
		           state == rtc::PeerConnection::State::Failed ||
		           state == rtc::PeerConnection::State::Closed) {
			m_connected = false;
			emit disconnected();
		}
	});
	
	m_peerConnection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
		m_dataChannel = dc;
		
		m_dataChannel->onMessage([this](auto data) {
			if (std::holds_alternative<rtc::binary>(data)) {
				auto binary = std::get<rtc::binary>(data);
				QByteArray audioData(reinterpret_cast<const char*>(binary.data()), 
				                     static_cast<int>(binary.size()));
				emit audioDataReceived(audioData);
			}
		});
	});
}

void WebRTCConnection::handleLocalDescription(const rtc::Description &description) {
	QString sdp = QString::fromStdString(std::string(description));
	
	if (description.type() == rtc::Description::Type::Offer) {
		emit offerCreated(sdp);
	} else if (description.type() == rtc::Description::Type::Answer) {
		emit answerCreated(sdp);
	}
}

void WebRTCConnection::handleLocalCandidate(const rtc::Candidate &candidate) {
	QString candidateStr = QString::fromStdString(std::string(candidate));
	QString sdpMid = QString::fromStdString(candidate.mid());
	
	emit iceCandidateGenerated(candidateStr, sdpMid, 0);
}

void WebRTCConnection::createOffer() {
	if (!m_peerConnection) {
		emit error("Peer connection not initialized");
		return;
	}
	
	try {
		// Create a data channel for audio
		m_dataChannel = m_peerConnection->createDataChannel("audio");
		
		m_dataChannel->onMessage([this](auto data) {
			if (std::holds_alternative<rtc::binary>(data)) {
				auto binary = std::get<rtc::binary>(data);
				QByteArray audioData(reinterpret_cast<const char*>(binary.data()), 
				                     static_cast<int>(binary.size()));
				emit audioDataReceived(audioData);
			}
		});
		
		// The offer will be created automatically and emitted via the onLocalDescription callback
	} catch (const std::exception& e) {
		emit error(QString("Failed to create offer: %1").arg(e.what()));
	}
}

void WebRTCConnection::createAnswer(const QString &offerSdp) {
	if (!m_peerConnection) {
		emit error("Peer connection not initialized");
		return;
	}
	
	try {
		rtc::Description offer(offerSdp.toStdString(), rtc::Description::Type::Offer);
		m_peerConnection->setRemoteDescription(offer);
		// The answer will be created automatically and emitted via the onLocalDescription callback
	} catch (const std::exception& e) {
		emit error(QString("Failed to create answer: %1").arg(e.what()));
	}
}

void WebRTCConnection::setRemoteDescription(const QString &sdp) {
	if (!m_peerConnection) {
		emit error("Peer connection not initialized");
		return;
	}
	
	try {
		rtc::Description description(sdp.toStdString(), rtc::Description::Type::Answer);
		m_peerConnection->setRemoteDescription(description);
	} catch (const std::exception& e) {
		emit error(QString("Failed to set remote description: %1").arg(e.what()));
	}
}

void WebRTCConnection::addIceCandidate(const QString &candidate, const QString &sdpMid, int /*sdpMLineIndex*/) {
	if (!m_peerConnection) {
		emit error("Peer connection not initialized");
		return;
	}
	
	try {
		rtc::Candidate rtcCandidate(candidate.toStdString(), sdpMid.toStdString());
		m_peerConnection->addRemoteCandidate(rtcCandidate);
	} catch (const std::exception& e) {
		emit error(QString("Failed to add ICE candidate: %1").arg(e.what()));
	}
}

void WebRTCConnection::sendAudioData(const QByteArray &data) {
	if (m_dataChannel && m_dataChannel->isOpen()) {
		try {
			std::vector<std::byte> binary(data.size());
			std::memcpy(binary.data(), data.data(), data.size());
			m_dataChannel->send(binary);
		} catch (const std::exception& e) {
			emit error(QString("Failed to send audio data: %1").arg(e.what()));
		}
	}
}

void WebRTCConnection::close() {
	if (m_dataChannel) {
		m_dataChannel->close();
		m_dataChannel.reset();
	}
	
	if (m_peerConnection) {
		m_peerConnection->close();
		m_peerConnection.reset();
	}
	
	m_connected = false;
}

#endif // USE_WEBRTC
