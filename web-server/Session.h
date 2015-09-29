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

#ifndef DCPLUSPLUS_DCPP_SESSION_H
#define DCPLUSPLUS_DCPP_SESSION_H

#include <web-server/stdinc.h>

#include <web-server/LazyInitWrapper.h>
#include <web-server/WebUser.h>

#include <api/ApiModule.h>

#include <airdcpp/GetSet.h>
#include <airdcpp/typedefs.h>

namespace webserver {
	// Sessions are owned by WebUserManager and WebSockets (websockets are closed when session is removed)
	class Session {
	public:
		Session(WebUserPtr& aUser, const std::string& aToken, bool aIsSecure);
		~Session();

		GETSET(uint64_t, lastActivity, LastActivity);
		const std::string& getToken() const noexcept {
			return token;
		}

		WebUserPtr getUser() {
			return user;
		}

		bool isSecure() const {
			return secure;
		}

		ApiModule* getModule(const std::string& aApiID);

		websocketpp::http::status_code::value handleRequest(ApiRequest& aRequest) throw(exception);

		Session(Session&) = delete;
		Session& operator=(Session&) = delete;
		//IGETSET(WebSocketPtr, socket, Socket, nullptr);

		void setSocket(const WebSocketPtr& aSocket) noexcept;
	private:
		typedef LazyInitWrapper<ApiModule> LazyModuleWrapper;
		std::map<std::string , LazyModuleWrapper> apiHandlers;

		const time_t started;
		const std::string  token;
		const bool secure;

		WebUserPtr user;
	};
}

#endif