// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "WebRTCHandler.h"

#ifdef USE_WEBRTC

#include "Server.h"
#include "ServerUser.h"
#include "src/shared/webrtc/WebRTCUtils.h"

WebRTCHandler::WebRTCHandler(Server *server, QObject *parent)
	: QObject(parent)
	, m_server(server) {
}

WebRTCHandler::~WebRTCHandler() {
	m_connections.clear();
}

bool WebRTCHandler::initialize(int /*port*/) {
	// TODO: Initialize WebSocket server for signaling
	// For now, we'll just prepare for peer connections
	return true;
}

void WebRTCHandler::setupPeerConnection(ServerUser *user) {
	if (!user) {
		return;
	}
	
	try {
		rtc::Configuration config;
		// TODO: Add ICE servers from configuration
		
		auto pc = std::make_shared<rtc::PeerConnection>(config);
		
		pc->onDataChannel([this, user](std::shared_ptr<rtc::DataChannel> dc) {
			dc->onMessage([this, user](auto data) {
				if (std::holds_alternative<rtc::binary>(data)) {
					auto binary = std::get<rtc::binary>(data);
					QByteArray audioData(reinterpret_cast<const char*>(binary.data()), 
					                     static_cast<int>(binary.size()));
					routeAudioToNative(user, audioData);
				}
			});
		});
		
		m_connections.insert(user->uiSession, pc);
	} catch (const std::exception& /*e*/) {
		// Error handling
	}
}

void WebRTCHandler::handleOffer(ServerUser *user, const QString &sdp) {
	if (!user) {
		return;
	}
	
	setupPeerConnection(user);
	
	auto it = m_connections.find(user->uiSession);
	if (it != m_connections.end()) {
		try {
			rtc::Description offer(sdp.toStdString(), rtc::Description::Type::Offer);
			it.value()->setRemoteDescription(offer);
			// Answer will be generated automatically
		} catch (const std::exception& /*e*/) {
			// Error handling
		}
	}
}

void WebRTCHandler::handleAnswer(ServerUser *user, const QString &sdp) {
	if (!user) {
		return;
	}
	
	auto it = m_connections.find(user->uiSession);
	if (it != m_connections.end()) {
		try {
			rtc::Description answer(sdp.toStdString(), rtc::Description::Type::Answer);
			it.value()->setRemoteDescription(answer);
		} catch (const std::exception& /*e*/) {
			// Error handling
		}
	}
}

void WebRTCHandler::handleIceCandidate(ServerUser *user, const QString &candidate, 
                                       const QString &sdpMid, int /*sdpMLineIndex*/) {
	if (!user) {
		return;
	}
	
	auto it = m_connections.find(user->uiSession);
	if (it != m_connections.end()) {
		try {
			rtc::Candidate rtcCandidate(candidate.toStdString(), sdpMid.toStdString());
			it.value()->addRemoteCandidate(rtcCandidate);
		} catch (const std::exception& /*e*/) {
			// Error handling
		}
	}
}

void WebRTCHandler::disconnectUser(ServerUser *user) {
	if (!user) {
		return;
	}
	
	auto it = m_connections.find(user->uiSession);
	if (it != m_connections.end()) {
		it.value()->close();
		m_connections.remove(user->uiSession);
	}
}

void WebRTCHandler::routeAudioToNative(ServerUser * /*user*/, const QByteArray & /*data*/) {
	// TODO: Convert WebRTC audio to Mumble format and route to native clients
}

void WebRTCHandler::routeAudioToWebRTC(ServerUser * /*user*/, const QByteArray & /*data*/) {
	// TODO: Convert Mumble audio to WebRTC format and send to WebRTC clients
}

#endif // USE_WEBRTC
