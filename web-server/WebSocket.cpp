/*
* Copyright (C) 2011-2015 AirDC++ Project
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <web-server/stdinc.h>
#include <web-server/WebSocket.h>

#include <airdcpp/Util.h>

namespace webserver {
	WebSocket::WebSocket(bool aIsSecure, websocketpp::connection_hdl aHdl) :
		secure(aIsSecure), hdl(aHdl) {

	}

	WebSocket::~WebSocket() {
		dcdebug("Websocket was deleted\n");
	}

	string WebSocket::getIp() const noexcept {
		if (secure) {
			auto conn = tlsServer->get_con_from_hdl(hdl);
			return conn->get_remote_endpoint();
		} else {
			auto conn = plainServer->get_con_from_hdl(hdl);
			return conn->get_remote_endpoint();
		}
	}

	void WebSocket::sendApiResponse(const json& aResponseJson, const json& aErrorJson, websocketpp::http::status_code::value aCode, int aCallbackId) {
		json j;
		j["callback_id"] = aCallbackId;
		j["code"] = aCode;

		if (aCode != websocketpp::http::status_code::ok) {
			dcdebug("Socket request %d failed: %s\n", aCallbackId, aErrorJson.dump().c_str());
			j["error"] = aErrorJson;
		} else if (!aResponseJson.is_null()) {
			j["data"] = aResponseJson;
		}

		sendPlain(j.dump(4));
	}

	void WebSocket::sendPlain(const string& aMsg) {
		//dcdebug("WebSocket::send: %s\n", aMsg.c_str());
		try {
			if (secure) {
				tlsServer->send(hdl, aMsg, websocketpp::frame::opcode::text);
			} else {
				plainServer->send(hdl, aMsg, websocketpp::frame::opcode::text);
			}

		} catch (const std::exception& e) {
			dcdebug("WebSocket::send failed: %s", e.what());
		}
	}

	void WebSocket::close(websocketpp::close::status::value aCode, const string& aMsg) {
		try {
			if (secure) {
				tlsServer->close(hdl, aCode, aMsg);
			} else {
				plainServer->close(hdl, aCode, aMsg);
			}
		} catch (const std::exception& e) {
			dcdebug("WebSocket::close failed: %s", e.what());
		}
	}
}