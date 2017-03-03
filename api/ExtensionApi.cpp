/*
* Copyright (C) 2011-2017 AirDC++ Project
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

#include <api/ExtensionApi.h>
#include <api/common/Serializer.h>

#include <web-server/JsonUtil.h>
#include <web-server/Extension.h>
#include <web-server/ExtensionManager.h>
#include <web-server/Session.h>
#include <web-server/WebServerManager.h>


#define EXTENSION_PARAM "extension"
namespace webserver {
	ExtensionApi::ExtensionApi(Session* aSession) : HookApiModule(aSession, Access::ADMIN, nullptr, Access::ADMIN), em(aSession->getServer()->getExtensionManager()) {
		em.addListener(this);

		METHOD_HANDLER(Access::ADMIN, METHOD_GET, (), ExtensionApi::handleGetExtensions);

		METHOD_HANDLER(Access::ADMIN, METHOD_POST, (), ExtensionApi::handleAddExtension);
		METHOD_HANDLER(Access::ADMIN, METHOD_GET, (STR_PARAM(EXTENSION_PARAM)), ExtensionApi::handleGetExtension);
		METHOD_HANDLER(Access::ADMIN, METHOD_DELETE, (STR_PARAM(EXTENSION_PARAM)), ExtensionApi::handleRemoveExtension);

		createSubscription("extension_added");
		createSubscription("extension_removed");

		createSubscription("extension_started");
		createSubscription("extension_stopped");
	}

	ExtensionApi::~ExtensionApi() {
		em.removeListener(this);
	}

	json ExtensionApi::serializeExtension(const ExtensionPtr& aExtension) noexcept {
		return {
			{ "name", aExtension->getName() },
			{ "version", aExtension->getVersion() },
		};
	}

	api_return ExtensionApi::handleGetExtension(ApiRequest& aRequest) {
		auto extension = em.getExtension(aRequest.getStringParam(EXTENSION_PARAM));
		if (!extension) {
			aRequest.setResponseErrorStr("Extension not found");
			return websocketpp::http::status_code::not_found;
		}

		aRequest.setResponseBody(serializeExtension(extension));
		return websocketpp::http::status_code::ok;
	}

	api_return ExtensionApi::handleAddExtension(ApiRequest& aRequest) {
		const auto& reqJson = aRequest.getRequestBody();

		auto url = JsonUtil::getField<string>("url", reqJson, false);
		auto sha = JsonUtil::getOptionalFieldDefault<string>("shasum", reqJson, Util::emptyString, true);

		if (!em.downloadExtension(url, sha)) {
			aRequest.setResponseErrorStr("Extension is being download already");
			return websocketpp::http::status_code::conflict;
		}

		return websocketpp::http::status_code::no_content;
	}

	api_return ExtensionApi::handleRemoveExtension(ApiRequest& aRequest) {
		auto extensionName = aRequest.getStringParam(EXTENSION_PARAM);

		auto extension = em.getExtension(extensionName);

		if (!extension) {
			aRequest.setResponseErrorStr("Extension not found");
			return websocketpp::http::status_code::not_found;
		}

		try {
			em.removeExtension(extension);
		} catch (const Exception& e) {
			aRequest.setResponseErrorStr(e.getError());
			return websocketpp::http::status_code::internal_server_error;
		}

		return websocketpp::http::status_code::no_content;
	}

	api_return ExtensionApi::handleGetExtensions(ApiRequest& aRequest) {
		aRequest.setResponseBody(Serializer::serializeList(em.getExtensions(), serializeExtension));
		return websocketpp::http::status_code::ok;
	}

	void ExtensionApi::on(ExtensionManagerListener::ExtensionAdded, const ExtensionPtr& aExtension) noexcept {
		maybeSend("extension_added", [&] {
			return serializeExtension(aExtension);
		});
	}

	void ExtensionApi::on(ExtensionManagerListener::ExtensionRemoved, const ExtensionPtr& aExtension) noexcept {
		maybeSend("extension_removed", [&] {
			return serializeExtension(aExtension);
		});
	}

	void ExtensionApi::on(ExtensionManagerListener::ExtensionStarted, const ExtensionPtr& aExtension) noexcept {
		maybeSend("extension_started", [&] {
			return serializeExtension(aExtension);
		});
	}

	void ExtensionApi::on(ExtensionManagerListener::ExtensionStopped, const ExtensionPtr& aExtension) noexcept {
		maybeSend("extension_stopped", [&] {
			return serializeExtension(aExtension);
		});
	}
}