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

#include <api/HubInfo.h>
#include <api/ApiModule.h>
#include <api/common/Serializer.h>

#include <web-server/JsonUtil.h>

namespace webserver {
	StringList HubInfo::subscriptionList = {
		"hub_updated",
		"hub_chat_message",
		"hub_status_message"
	};

	HubInfo::HubInfo(ParentType* aParentModule, const ClientPtr& aClient) :
		SubApiModule(aParentModule, aClient->getClientId(), subscriptionList), client(aClient) {

		client->addListener(this);

		METHOD_HANDLER("messages", ApiRequest::METHOD_GET, (NUM_PARAM), false, HubInfo::handleGetMessages);
		METHOD_HANDLER("message", ApiRequest::METHOD_POST, (), true, HubInfo::handlePostMessage);

		METHOD_HANDLER("reconnect", ApiRequest::METHOD_POST, (), false, HubInfo::handleReconnect);
		METHOD_HANDLER("favorite", ApiRequest::METHOD_POST, (), false, HubInfo::handleFavorite);
		METHOD_HANDLER("password", ApiRequest::METHOD_POST, (), true, HubInfo::handlePassword);
		METHOD_HANDLER("redirect", ApiRequest::METHOD_POST, (), false, HubInfo::handleRedirect);

		METHOD_HANDLER("read", ApiRequest::METHOD_POST, (), false, HubInfo::handleSetRead);
	}

	HubInfo::~HubInfo() {
		client->removeListener(this);
	}

	api_return HubInfo::handleReconnect(ApiRequest& aRequest) throw(exception) {
		client->reconnect();
		return websocketpp::http::status_code::ok;
	}

	api_return HubInfo::handleFavorite(ApiRequest& aRequest) throw(exception) {
		if (!client->saveFavorite()) {
			aRequest.setResponseErrorStr(STRING(FAVORITE_HUB_ALREADY_EXISTS));
			return websocketpp::http::status_code::bad_request;
		}

		return websocketpp::http::status_code::ok;
	}

	api_return HubInfo::handlePassword(ApiRequest& aRequest) throw(exception) {
		auto password = JsonUtil::getField<string>("password", aRequest.getRequestBody(), false);

		client->password(password);
		return websocketpp::http::status_code::ok;
	}

	api_return HubInfo::handleRedirect(ApiRequest& aRequest) throw(exception) {
		client->doRedirect();
		return websocketpp::http::status_code::ok;
	}

	json HubInfo::serializeIdentity(const ClientPtr& aClient) noexcept {
		return{
			{ "name", aClient->getHubName() },
			{ "description", aClient->getHubDescription() },
			{ "user_count", aClient->getUserCount() },
			{ "share_size", aClient->getTotalShare() },
		};
	}

	json HubInfo::serializeConnectState(const ClientPtr& aClient) noexcept {
		if (!aClient->getRedirectUrl().empty()) {
			return{
				{ "id", "redirect" },
				{ "hub_url", aClient->getRedirectUrl() }
			};
		}

		string id;
		switch (aClient->getConnectState()) {
			case Client::STATE_CONNECTING:
			case Client::STATE_PROTOCOL:
			case Client::STATE_IDENTIFY: id = "connecting"; break;
			case Client::STATE_VERIFY:  id = "password"; break;
			case Client::STATE_NORMAL: id = "connected"; break;
			case Client::STATE_DISCONNECTED: id = "disconnected"; break;
		}

		return {
			{ "id", id }
		};
	}

	api_return HubInfo::handleSetRead(ApiRequest& aRequest) throw(exception) {
		client->setRead();
		return websocketpp::http::status_code::ok;
	}

	api_return HubInfo::handleGetMessages(ApiRequest& aRequest) throw(exception) {
		auto j = Serializer::serializeFromEnd(
			aRequest.getRangeParam(0),
			client->getCache().getMessages(),
			Serializer::serializeMessage);

		aRequest.setResponseBody(j);
		return websocketpp::http::status_code::ok;
	}

	api_return HubInfo::handlePostMessage(ApiRequest& aRequest) throw(exception) {
		decltype(auto) requestJson = aRequest.getRequestBody();

		auto message = JsonUtil::getField<string>("message", requestJson, false);
		auto thirdPerson = JsonUtil::getOptionalField<bool>("third_person", requestJson);

		string error_;
		if (!client->hubMessage(message, error_, thirdPerson ? *thirdPerson : false)) {
			aRequest.setResponseErrorStr(error_);
			return websocketpp::http::status_code::internal_server_error;
		}

		return websocketpp::http::status_code::ok;
	}

	void HubInfo::on(ClientListener::ChatMessage, const Client*, const ChatMessagePtr& aMessage) noexcept {
		if (!aMessage->getRead()) {
			sendUnread();
		}

		if (!subscriptionActive("hub_chat_message")) {
			return;
		}

		send("hub_chat_message", Serializer::serializeChatMessage(aMessage));
	}

	void HubInfo::on(ClientListener::StatusMessage, const Client*, const LogMessagePtr& aMessage, int aFlags) noexcept {
		if (!subscriptionActive("hub_status_message")) {
			return;
		}

		send("hub_status_message", Serializer::serializeLogMessage(aMessage));
	}

	void HubInfo::on(ClientListener::Disconnecting, const Client*) noexcept {

	}

	void HubInfo::on(ClientListener::Redirected, const string&, const ClientPtr& aNewClient) noexcept {
		client->removeListener(this);
		client = aNewClient;
		aNewClient->addListener(this);

		sendConnectState();
	}

	/*void HubInfo::on(ClientListener::Connecting, const Client*) noexcept {
		sendConnectState();
	}

	void HubInfo::on(ClientListener::Connected, const Client*) noexcept {
		sendConnectState();
	}*/

	void HubInfo::on(Failed, const string&, const string&) noexcept {
		sendConnectState();
	}

	void HubInfo::on(ClientListener::Redirect, const Client*, const string&) noexcept {
		sendConnectState();
	}

	void HubInfo::on(ConnectStateChanged, const Client*, uint8_t aState) noexcept {
		if (aState == Client::STATE_IDENTIFY || aState == Client::STATE_PROTOCOL) {
			// Use the old "connecting" state still
			return;
		}

		sendConnectState();
	}

	void HubInfo::on(ClientListener::GetPassword, const Client*) noexcept {
		sendConnectState();
	}

	void HubInfo::on(ClientListener::HubUpdated, const Client*) noexcept {
		onHubUpdated({
			{ "identity", serializeIdentity(client) }
		});
	}

	void HubInfo::on(ClientListener::HubTopic, const Client*, const string&) noexcept {

	}

	void HubInfo::on(ClientListener::MessagesRead, const Client*) noexcept {
		sendUnread();
	}

	void HubInfo::sendConnectState() noexcept {
		onHubUpdated({
			{ "connect_state", serializeConnectState(client) }
		});
	}

	void HubInfo::sendUnread() noexcept {
		onHubUpdated({
			{ "unread_count", client->getCache().countUnread(Message::TYPE_CHAT) }
		});
	}

	void HubInfo::onHubUpdated(const json& aData) noexcept {
		if (!subscriptionActive("hub_updated")) {
			return;
		}

		send("hub_updated", aData);
	}
}