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

#ifndef DCPLUSPLUS_DCPP_FILELISTAPI_H
#define DCPLUSPLUS_DCPP_FILELISTAPI_H

#include <web-server/stdinc.h>

#include <api/HierarchicalApiModule.h>
#include <api/FilelistInfo.h>

#include <airdcpp/typedefs.h>
#include <airdcpp/DirectoryListingManager.h>

namespace webserver {
	class FilelistApi : public ParentApiModule<CID, FilelistInfo>, private DirectoryListingManagerListener {
	public:
		static StringList subscriptionList;

		FilelistApi(Session* aSession);
		~FilelistApi();

		int getVersion() const noexcept {
			return 0;
		}

	private:
		void addList(const DirectoryListingPtr& aList) noexcept;

		api_return handlePostList(ApiRequest& aRequest) throw(exception);
		api_return handleDeleteList(ApiRequest& aRequest) throw(exception);

		api_return handleGetLists(ApiRequest& aRequest) throw(exception);

		void on(DirectoryListingManagerListener::ListingCreated, const DirectoryListingPtr& aList) noexcept;
		//void on(DirectoryListingManagerListener::OpenListing, const DirectoryListingPtr& aList, const string& aDir, const string& aXML) noexcept;
		void on(DirectoryListingManagerListener::ListingClosed, const DirectoryListingPtr&) noexcept;
		//void on(MessageManagerListener::ChatRemoved, const PrivateChatPtr& aChat) noexcept;

		static json serializeList(const DirectoryListingPtr& aList) noexcept;
	};
}

#endif