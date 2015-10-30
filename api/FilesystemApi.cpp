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

#include <api/FilesystemApi.h>
#include <api/common/Serializer.h>

#include <web-server/JsonUtil.h>

#include <airdcpp/File.h>

#ifdef WIN32
#include <api/platform/windows/Filesystem.h>
#endif

namespace webserver {
	FilesystemApi::FilesystemApi(Session* aSession) : ApiModule(aSession) {
		METHOD_HANDLER("list_items", ApiRequest::METHOD_POST, (), true, FilesystemApi::handleListItems);
		METHOD_HANDLER("directory", ApiRequest::METHOD_POST, (), true, FilesystemApi::handlePostDirectory);
	}

	FilesystemApi::~FilesystemApi() {
	}

	api_return FilesystemApi::handleListItems(ApiRequest& aRequest) throw(exception) {
		const auto& reqJson = aRequest.getRequestBody();
		auto path = JsonUtil::getField<string>("path", reqJson, WIN32 ? true : false);

		auto dirsOnly = JsonUtil::getOptionalField<bool>("directories_only", reqJson);

		json retJson;
		if (path.empty()) {
#ifdef WIN32
			retJson = Filesystem::getDriveListing(false);
#endif
		} else {
			try {
				retJson = serializeDirectoryContent(path, dirsOnly ? *dirsOnly : false);
			} catch (const FileException& e) {
				aRequest.setResponseErrorStr("Failed to get directory content: " + e.getError());
				return websocketpp::http::status_code::internal_server_error;
			}
		}

		if (retJson.is_null()) {
			retJson = json::array();
		}

		aRequest.setResponseBody(retJson);
		return websocketpp::http::status_code::ok;
	}

	json FilesystemApi::serializeDirectoryContent(const string& aPath, bool aDirectoriesOnly) {
		json retJson;

		FileFindIter end;
		for (FileFindIter i(aPath, "*"); i != end; ++i) {
			auto fileName = i->getFileName();
			if (fileName == "." || fileName == "..") {
				continue;
			}

			if (aDirectoriesOnly && !i->isDirectory()) {
				continue;
			}

			json item;
			item["name"] = fileName;
			if (i->isDirectory()) {
				item["type"] = Serializer::serializeFolderType(-1, -1);
			} else {
				item["type"] = Serializer::serializeFileType(i->getFileName());
				item["size"] = i->getSize();
			}

			retJson.push_back(item);
		}

		return retJson;
	}

	api_return FilesystemApi::handlePostDirectory(ApiRequest& aRequest) throw(exception) {
		const auto& reqJson = aRequest.getRequestBody();

		auto path = JsonUtil::getField<string>("path", reqJson, false);
		try {
			File::createDirectory(path);
		} catch (const FileException& e) {
			aRequest.setResponseErrorStr("Failed to create directory: " + e.getError());
			return websocketpp::http::status_code::internal_server_error;
		}

		return websocketpp::http::status_code::ok;
	}
}