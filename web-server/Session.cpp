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
#include <web-server/Session.h>
#include <web-server/ApiRequest.h>

#include <api/FavoriteDirectoryApi.h>
#include <api/FavoriteHubApi.h>
#include <api/FilelistApi.h>
#include <api/HistoryApi.h>
#include <api/HubApi.h>
#include <api/LogApi.h>
#include <api/PrivateChatApi.h>
#include <api/QueueApi.h>
#include <api/SearchApi.h>
#include <api/ShareApi.h>
#include <api/TransferApi.h>

#include <airdcpp/TimerManager.h>

namespace webserver {
#define ADD_MODULE(name, type) (apiHandlers.emplace(name, LazyModuleWrapper([this] { return make_unique<type>(this); })))

	Session::Session(WebUserPtr& aUser, const string& aToken, bool aIsSecure) : 
		user(aUser), token(aToken), started(GET_TICK()), lastActivity(lastActivity), secure(aIsSecure) {

		ADD_MODULE("favorite_directories", FavoriteDirectoryApi);
		ADD_MODULE("favorite_hubs", FavoriteHubApi);
		ADD_MODULE("filelists", FilelistApi);
		ADD_MODULE("histories", HistoryApi);
		ADD_MODULE("hubs", HubApi);
		ADD_MODULE("log", LogApi);
		ADD_MODULE("private_chat", PrivateChatApi);
		ADD_MODULE("queue", QueueApi);
		ADD_MODULE("search", SearchApi);
		ADD_MODULE("share", ShareApi);
		ADD_MODULE("transfers", TransferApi);
	}

	Session::~Session() {
		dcdebug("Session %s was deleted\n", token.c_str());
	}

	ApiModule* Session::getModule(const string& aModule) {
		auto h = apiHandlers.find(aModule);
		return h != apiHandlers.end() ? h->second.get() : nullptr;
	}

	websocketpp::http::status_code::value Session::handleRequest(ApiRequest& aRequest) throw(exception) {
		auto h = apiHandlers.find(aRequest.getApiModule());
		if (h != apiHandlers.end()) {
			if (aRequest.getApiVersion() != h->second->getVersion()) {
				aRequest.setResponseErrorStr("Invalid API version");
				return websocketpp::http::status_code::precondition_failed;
			}

			return h->second->handleRequest(aRequest);
		}

		aRequest.setResponseErrorStr("Section not found");
		return websocketpp::http::status_code::not_found;
	}

	void Session::onSocketConnected(const WebSocketPtr& aSocket) noexcept {
		fire(SessionListener::SocketConnected(), aSocket);
	}

	void Session::onSocketDisconnected() noexcept {
		fire(SessionListener::SocketDisconnected());
	}
}