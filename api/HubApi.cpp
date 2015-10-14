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

#include <api/HubApi.h>

#include <api/common/Serializer.h>

#include <web-server/JsonUtil.h>

#include <airdcpp/ClientManager.h>
#include <airdcpp/HubEntry.h>

namespace webserver {
	StringList HubApi::subscriptionList = {
		"hub_created",
		"hub_removed"
	};

	HubApi::HubApi(Session* aSession) : ParentApiModule("session", TOKEN_PARAM, aSession, subscriptionList, HubInfo::subscriptionList, [](const string& aId) { return Util::toUInt32(aId); }) {
		ClientManager::getInstance()->addListener(this);

		METHOD_HANDLER("sessions", ApiRequest::METHOD_GET, (), false, HubApi::handleGetHubs);

		METHOD_HANDLER("session", ApiRequest::METHOD_POST, (), true, HubApi::handleConnect);
		METHOD_HANDLER("session", ApiRequest::METHOD_DELETE, (TOKEN_PARAM), false, HubApi::handleDisconnect);

		METHOD_HANDLER("search_nicks", ApiRequest::METHOD_POST, (), true, HubApi::handleSearchNicks);

		auto rawHubs = ClientManager::getInstance()->getClients();
		for (const auto& c : rawHubs | map_values) {
			addHub(c);
		}
	}

	HubApi::~HubApi() {
		ClientManager::getInstance()->removeListener(this);
	}

	json HubApi::serializeClient(const ClientPtr& aClient) noexcept {
		return{
			{ "identity", HubInfo::serializeIdentity(aClient) },
			{ "connect_state", HubInfo::serializeConnectState(aClient) },
			{ "unread_count", aClient->getCache().countUnreadChatMessages() },
			{ "hub_url", aClient->getHubUrl() },
			{ "id", aClient->getClientId() },
			{ "favorite_hub", aClient->getFavToken() },
			{ "share_profile", aClient->getShareProfile() }
			//{ "share_profile", Serializer::serializeShare aClient->getShareProfile() },
		};
	}

	void HubApi::addHub(const ClientPtr& aClient) noexcept {
		auto hubInfo = unique_ptr<HubInfo>(new HubInfo(this, aClient));

		{
			WLock l(cs);
			subModules.emplace(aClient->getClientId(), move(hubInfo));
		}
	}

	api_return HubApi::handleGetHubs(ApiRequest& aRequest) throw(exception) {
		json retJson;

		{
			RLock l(cs);
			if (!subModules.empty()) {
				for (const auto& c : subModules | map_values) {
					retJson.push_back(serializeClient(c->getClient()));
				}
			} else {
				retJson = json::array();
			}
		}

		aRequest.setResponseBody(retJson);
		return websocketpp::http::status_code::ok;
	}

	void HubApi::on(ClientManagerListener::ClientCreated, const ClientPtr& aClient) noexcept {
		addHub(aClient);
		if (!subscriptionActive("hub_created")) {
			return;
		}

		send("hub_created", serializeClient(aClient));
	}

	void HubApi::on(ClientManagerListener::ClientRemoved, const ClientPtr& aClient) noexcept {
		{
			WLock l(cs);
			subModules.erase(aClient->getClientId());
		}

		if (!subscriptionActive("hub_removed")) {
			return;
		}

		send("hub_removed", {
			{ "id", aClient->getClientId() }
		});
	}

	api_return HubApi::handleConnect(ApiRequest& aRequest) throw(exception) {
		decltype(auto) reqJson = aRequest.getRequestBody();

		auto address = JsonUtil::getField<string>("hub_url", reqJson, false);

		RecentHubEntryPtr r = new RecentHubEntry(address);
		auto client = ClientManager::getInstance()->createClient(r, SETTING(DEFAULT_SP));
		if (!client) {
			aRequest.setResponseErrorStr("Hub exists");
			return websocketpp::http::status_code::bad_request;
		}

		aRequest.setResponseBody({
			{ "id", client->getClientId() }
		});

		return websocketpp::http::status_code::ok;
	}

	api_return HubApi::handleDisconnect(ApiRequest& aRequest) throw(exception) {
		if (!ClientManager::getInstance()->putClient(aRequest.getTokenParam(0))) {
			return websocketpp::http::status_code::not_found;
		}

		return websocketpp::http::status_code::ok;
	}

	api_return HubApi::handleSearchNicks(ApiRequest& aRequest) throw(exception) {
		decltype(auto) reqJson = aRequest.getRequestBody();

		auto pattern = JsonUtil::getField<string>("pattern", reqJson);
		auto maxResults = JsonUtil::getField<size_t>("max_results", reqJson);

		auto users = ClientManager::getInstance()->searchNicks(pattern, maxResults);

		json retJson;
		for (const auto& u : users) {
			retJson.push_back(Serializer::serializeOnlineUser(u));
		}

		aRequest.setResponseBody(retJson);
		return websocketpp::http::status_code::ok;
	}
}