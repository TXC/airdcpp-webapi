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

#ifndef DCPLUSPLUS_DCPP_PRIVATEMESSAGE_H
#define DCPLUSPLUS_DCPP_PRIVATEMESSAGE_H

#include <web-server/stdinc.h>

#include <airdcpp/typedefs.h>
#include <airdcpp/GetSet.h>

#include <airdcpp/ChatMessage.h>
#include <airdcpp/User.h>

#include <api/ApiModule.h>

namespace webserver {
	class PrivateChatInfo : public ApiModule {
	public:
		json serialize() const noexcept;

		typedef shared_ptr<PrivateChatInfo> Ptr;
		typedef vector<Ptr> List;
		typedef unordered_map<CID, Ptr> Map;

		PrivateChatInfo(Session* aSession, const PrivateChatPtr& aChat);
		~PrivateChatInfo() {	}

		//const UserPtr& getUser() const { return sr->getUser().user; }
		//const string& getHubUrl() const { return sr->getUser().hint; }

		PrivateChatPtr getChat() const noexcept { return chat; }
	private:
		//PrivateMessage::List getCurrentViewItems();

		PrivateChatPtr chat;
	};

	typedef PrivateChatInfo::Ptr PrivateChatInfoPtr;
}

#endif