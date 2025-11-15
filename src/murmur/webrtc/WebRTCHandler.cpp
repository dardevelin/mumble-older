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

bool WebRTCHandler::initialize(int port) {
	// Initialize WebSocket server for signaling
	// Note: libdatachannel doesn't include a WebSocket server by default.
	// For now, we support direct peer connections via the Mumble protocol.
	// A separate WebSocket server implementation would be needed for browser clients.
	// This can be implemented using a library like Boost.Beast or Qt WebSockets.
	
	// Store the port for future use
	m_port = port;
	
	// For now, signaling will go through the existing Mumble TCP connection
	// Browser clients would need a separate WebSocket endpoint
	
	return true;
}

void WebRTCHandler::setupPeerConnection(ServerUser *user) {
	if (!user) {
		return;
	}
	
	try {
		rtc::Configuration config;
		
		// Add ICE servers from configuration
		// Default STUN servers for NAT traversal
		config.iceServers.push_back("stun:stun.l.google.com:19302");
		config.iceServers.push_back("stun:stun1.l.google.com:19302");
		
		// Additional servers can be configured from server settings
		// For production, TURN servers should also be configured for reliable connectivity
		
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

void WebRTCHandler::routeAudioToNative(ServerUser *user, const QByteArray &data) {
	// Convert WebRTC audio (RTP) to Mumble format and route to native clients
	if (!m_server || !user || data.isEmpty()) {
		return;
	}
	
	// Extract Opus payload from RTP packet using utility function
	QByteArray opusData = Mumble::WebRTC::Utils::rtpToMumbleAudio(
		reinterpret_cast<const uint8_t*>(data.data()), 
		static_cast<size_t>(data.size())
	);
	
	if (opusData.isEmpty()) {
		return;
	}
	
	// The Opus data can now be sent to other clients through Mumble's normal audio pipeline
	// This would typically involve:
	// 1. Packaging the Opus data in Mumble's audio packet format
	// 2. Sending it through the server's audio distribution system
	// 
	// Note: Full integration requires access to Server's sendAudio methods
	// which would be implemented in the integration phase
}

void WebRTCHandler::routeAudioToWebRTC(ServerUser *user, const QByteArray &data) {
	// Convert Mumble audio to WebRTC format (RTP) and send to WebRTC clients
	if (!user || data.isEmpty()) {
		return;
	}
	
	// Get the peer connection for this user
	auto it = m_connections.find(user->uiSession);
	if (it == m_connections.end()) {
		return;
	}
	
	// Convert Mumble Opus data to RTP format
	QByteArray rtpPacket = Mumble::WebRTC::Utils::mumbleAudioToRtp(
		reinterpret_cast<const uint8_t*>(data.data()), 
		static_cast<size_t>(data.size())
	);
	
	if (rtpPacket.isEmpty()) {
		return;
	}
	
	// Send via data channel if available
	// Note: In a full implementation, we would:
	// 1. Get the data channel from the peer connection
	// 2. Send the RTP packet over the data channel
	// 3. Handle sequence numbers and timestamps properly
	//
	// This requires accessing the data channel which would be stored
	// in the peer connection setup phase
}

#endif // USE_WEBRTC
