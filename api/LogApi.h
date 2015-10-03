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

#ifndef DCPLUSPLUS_DCPP_LOGAPI_H
#define DCPLUSPLUS_DCPP_LOGAPI_H

#include <web-server/stdinc.h>

#include <api/ApiModule.h>

#include <airdcpp/typedefs.h>
#include <airdcpp/LogManagerListener.h>

namespace webserver {
	class LogApi : public ApiModule, private LogManagerListener {
	public:
		LogApi(Session* aSession);
		~LogApi();

		int getVersion() const noexcept {
			return 0;
		}
		//websocketpp::http::status_code::value handleRequest(ApiRequest& aRequest, const SessionPtr& aSession, const json& aJson, std::string& output_) throw(exception);
	private:
		static json serializeMessage(const LogMessage& aMessageData) noexcept;
		api_return handleGetLog(ApiRequest& aRequest)  throw(exception);

		// LogManagerListener
		void on(LogManagerListener::Message, const LogMessage& aMessageData) noexcept;
	};
}

#endif