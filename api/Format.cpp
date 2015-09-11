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

#include "Format.h"

#include <client/ResourceManager.h>
#include <client/ClientManager.h>

#include <boost/range/algorithm/copy.hpp>

namespace webserver {
	std::string Format::formatFolderContent(int fileCount, int folderCount) noexcept {
		std::string name;

		bool hasFileInfo = fileCount > 0;
		bool hasFolderInfo = folderCount > 0;
		//if (hasFileInfo || hasFolderInfo)
		//	name += _T(" (");

		if (hasFolderInfo) {
			name += STRING_F(X_FOLDERS, folderCount);
		}

		if (hasFileInfo) {
			if (hasFolderInfo)
				name += ", ";
			name += STRING_F(X_FILES, fileCount);
		}

		//if (hasFileInfo || hasFolderInfo)
		//	name += _T(")");

		return name;
	}

	std::string Format::formatFileType(const string& aPath) noexcept {
		auto type = Util::getFileExt(aPath);
		if (type.size() > 0 && type[0] == '.') {
			type.erase(0, 1);
		}

		return type;
	}

	std::string Format::formatNicks(const HintedUser& aUser) noexcept {
		return Util::listToString(ClientManager::getInstance()->getNicks(aUser));
	}

	std::string Format::formatHubs(const HintedUser& aUser) noexcept {
		return Util::listToString(ClientManager::getInstance()->getHubNames(aUser));
	}
}