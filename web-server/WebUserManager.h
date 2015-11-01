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

#ifndef DCPLUSPLUS_DCPP_WEBUSERMANAGER_H
#define DCPLUSPLUS_DCPP_WEBUSERMANAGER_H

#include <web-server/stdinc.h>

#include <airdcpp/CriticalSection.h>

#include <web-server/Session.h>
#include <web-server/Timer.h>
#include <web-server/WebServerManagerListener.h>
#include <web-server/WebUser.h>

namespace webserver {
	class WebServerManager;
	class WebUserManager : private WebServerManagerListener {
	public:
		WebUserManager(WebServerManager* aServer);
		~WebUserManager();

		SessionPtr authenticate(const string& aUserName, const string& aPassword, bool aIsSecure) noexcept;

		SessionPtr getSession(const string& aSession) const noexcept;
		void logout(const SessionPtr& aSession);

		bool hasUsers() const noexcept;
		bool hasUser(const string& aUserName) const noexcept;

		// Adds a new users or updates password for an existing one. Returns false when an existing user was found.
		bool addUser(const string& aUserName, const string& aPassword) noexcept;

		bool removeUser(const string& aUserName) noexcept;
		StringList getUserNames() const noexcept;
	private:
		mutable SharedMutex cs;

		std::map<std::string, WebUserPtr> users;
		std::map<std::string, SessionPtr> sessions;

		void checkExpiredSessions() noexcept;
		TimerPtr expirationTimer;

		void on(WebServerManagerListener::Started) noexcept;
		void on(WebServerManagerListener::LoadSettings, SimpleXML& aXml) noexcept;
		void on(WebServerManagerListener::SaveSettings, SimpleXML& aXml) noexcept;

		WebServerManager* server;
	};
}

#endif