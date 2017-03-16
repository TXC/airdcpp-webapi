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

#include <airdcpp/File.h>


#define EXTENSION_PARAM_ID "extension"
#define EXTENSION_PARAM (ApiModule::RequestHandler::Param(EXTENSION_PARAM_ID, regex(R"(^airdcpp-.+$)")))
namespace webserver {
	StringList ExtensionApi::subscriptionList = {
		"extension_added",
		"extension_removed",
		"extension_updated",
	};

	ExtensionApi::ExtensionApi(Session* aSession) : /*HookApiModule(aSession, Access::ADMIN, nullptr, Access::ADMIN),*/ 
		em(aSession->getServer()->getExtensionManager()),
		ParentApiModule(EXTENSION_PARAM, Access::ADMIN, aSession, ExtensionApi::subscriptionList,
			ExtensionInfo::subscriptionList,
			[](const string& aId) { return aId; },
			[](const ExtensionInfo& aInfo) { return serializeExtension(aInfo.getExtension()); }
		)
	{
		em.addListener(this);

		METHOD_HANDLER(Access::ADMIN, METHOD_POST, (), ExtensionApi::handlePostExtension);
		METHOD_HANDLER(Access::ADMIN, METHOD_POST, (EXACT_PARAM("download")), ExtensionApi::handleDownloadExtension);
		METHOD_HANDLER(Access::ADMIN, METHOD_DELETE, (EXTENSION_PARAM), ExtensionApi::handleRemoveExtension);

		for (const auto& ext: em.getExtensions()) {
			addExtension(ext);
		}
	}

	ExtensionApi::~ExtensionApi() {
		em.removeListener(this);
	}

	void ExtensionApi::addExtension(const ExtensionPtr& aExtension) noexcept {
		addSubModule(aExtension->getName(), std::make_shared<ExtensionInfo>(this, aExtension));
	}

	json ExtensionApi::serializeExtension(const ExtensionPtr& aExtension) noexcept {
		return {
			{ "id", aExtension->getName() },
			{ "name", aExtension->getName() },
			{ "description", aExtension->getDescription() },
			{ "version", aExtension->getVersion() },
			{ "homepage", aExtension->getHomepage() },
			{ "author", aExtension->getAuthor() },
			{ "running", aExtension->isRunning() },
			{ "private", aExtension->isPrivate() },
			{ "logs", ExtensionInfo::serializeLogs(aExtension) },
			{ "engines", aExtension->getEngines() },
			{ "managed", aExtension->isManaged() },
			{ "has_settings", aExtension->hasSettings() },
		};
	}

	ExtensionPtr ExtensionApi::getExtension(ApiRequest& aRequest) {
		auto extension = em.getExtension(aRequest.getStringParam(EXTENSION_PARAM_ID));
		if (!extension) {
			throw RequestException(websocketpp::http::status_code::not_found, "Extension not found");
		}

		return extension;
	}

	api_return ExtensionApi::handlePostExtension(ApiRequest& aRequest) {
		const auto& reqJson = aRequest.getRequestBody();

		try {
			em.registerRemoteExtension(aRequest.getSession(), reqJson);
		} catch (const Exception& e) {
			aRequest.setResponseErrorStr(e.getError());
			return websocketpp::http::status_code::bad_request;
		}

		return websocketpp::http::status_code::no_content;
	}

	api_return ExtensionApi::handleDownloadExtension(ApiRequest& aRequest) {
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
		auto extension = getExtension(aRequest);

		try {
			em.removeExtension(extension);
		} catch (const Exception& e) {
			aRequest.setResponseErrorStr(e.getError());
			return websocketpp::http::status_code::internal_server_error;
		}

		return websocketpp::http::status_code::no_content;
	}

	void ExtensionApi::on(ExtensionManagerListener::ExtensionAdded, const ExtensionPtr& aExtension) noexcept {
		addExtension(aExtension);

		maybeSend("extension_added", [&] {
			return serializeExtension(aExtension);
		});
	}

	void ExtensionApi::on(ExtensionManagerListener::ExtensionRemoved, const ExtensionPtr& aExtension) noexcept {
		removeSubModule(aExtension->getName());
		maybeSend("extension_removed", [&] {
			return serializeExtension(aExtension);
		});
	}

	void ExtensionApi::on(ExtensionManagerListener::ExtensionUpdated, const ExtensionPtr& aExtension) noexcept {
		maybeSend("extension_updated", [&] {
			return serializeExtension(aExtension);
		});
	}
}