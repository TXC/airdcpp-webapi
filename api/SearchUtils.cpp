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

#include <api/SearchUtils.h>
#include <api/SearchApi.h>

#include <api/common/Format.h>

namespace webserver {
	json SearchUtils::serializeResult(const SearchApi::SearchInfoPtr& aResult, int aPropertyName) noexcept {
		json j;

		switch (aPropertyName) {
		case SearchApi::PROP_TYPE:
		{
			if (aResult->sr->getType() == SearchResult::TYPE_FILE) {
				return Serializer::serializeFileType(aResult->sr->getPath());
			} else {
				return Serializer::serializeFolderType(aResult->sr->getFileCount(), aResult->sr->getFolderCount());
			}
		}
		case SearchApi::PROP_SLOTS: {
			json j;
			j["str"] = aResult->sr->getSlotString();
			j["free"] = aResult->sr->getFreeSlots();
			j["total"] = aResult->sr->getSlots();
			return j;
		}
		}

		return j;
	}

	int SearchUtils::compareResults(const SearchApi::SearchInfoPtr& a, const SearchApi::SearchInfoPtr& b, int aPropertyName) noexcept {
		switch (aPropertyName) {
		case SearchApi::PROP_NAME: {
			if (a->sr->getType() == b->sr->getType())
				return Util::stricmp(a->sr->getFileName().c_str(), b->sr->getFileName().c_str());
			else
				return (a->sr->getType() == SearchResult::TYPE_DIRECTORY) ? -1 : 1;
		}
		case SearchApi::PROP_TYPE: {
			if (a->sr->getType() != b->sr->getType()) {
				// Directories go first
				return a->sr->getType() == SearchResult::TYPE_FILE ? 1 : -1;
			}

			if (a->sr->getType() != SearchResult::TYPE_FILE && b->sr->getType() != SearchResult::TYPE_FILE) {
				// Directory bundles
				auto dirsA = a->sr->getFolderCount();
				auto dirsB = b->sr->getFolderCount();
				if (dirsA != dirsB) {
					return compare(dirsA, dirsB);
				}

				auto filesA = a->sr->getFileCount();
				auto filesB = b->sr->getFileCount();

				return compare(filesA, filesB);
			}

			return Util::stricmp(Util::getFileExt(a->sr->getPath()), Util::getFileExt(b->sr->getPath()));
		}
		case SearchApi::PROP_SLOTS: {
			if (a->sr->getFreeSlots() == b->sr->getFreeSlots())
				return compare(a->sr->getSlots(), b->sr->getSlots());
			else
				return compare(a->sr->getFreeSlots(), b->sr->getFreeSlots());
		}
		default:
			dcassert(0);
		}

		return 0;
	}
	std::string SearchUtils::getStringInfo(const SearchApi::SearchInfoPtr& aResult, int aPropertyName) noexcept {
		switch (aPropertyName) {
		case SearchApi::PROP_NAME: return aResult->sr->getFileName();
		case SearchApi::PROP_PATH: return aResult->sr->getPath();
		case SearchApi::PROP_USERS: return Format::formatNicks(aResult->sr->getUser());
		case SearchApi::PROP_TYPE: {
			if (aResult->sr->getType() == SearchResult::TYPE_DIRECTORY) {
				return Format::formatFolderContent(aResult->sr->getFileCount(), aResult->sr->getFolderCount());
			}
			else {
				return Format::formatFileType(aResult->sr->getPath());
			}
		}
		case SearchApi::PROP_SLOTS: return aResult->sr->getSlotString();
		default: dcassert(0); return 0;
		}
	}
	double SearchUtils::getNumericInfo(const SearchApi::SearchInfoPtr& aResult, int aPropertyName) noexcept {
		switch (aPropertyName) {
		case SearchApi::PROP_SIZE: return (double)aResult->sr->getSize();
		case SearchApi::PROP_HITS: return (double)aResult->hits;
		case SearchApi::PROP_CONNECTION: return (double)aResult->sr->getConnectionInt();
		case SearchApi::PROP_RELEVANCY : return aResult->getTotalRelevancy();
		case SearchApi::PROP_DATE: return (double)aResult->sr->getDate();
		default: dcassert(0); return 0;
		}
	}
}