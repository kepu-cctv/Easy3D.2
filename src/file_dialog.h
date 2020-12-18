/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _EASY3D_FILE_DIALOG_H_
#define _EASY3D_FILE_DIALOG_H_

#include <string>
#include <vector>

namespace easy3d
{
	/**
	 * \brief Open a native file open/save dialog.
	 *
	 * \param filetypes
	 *     Pairs of permissible formats with descriptions like
	 *     ``("png", "Portable Network Graphics")``.
	 *
	 * \param save
	 *     Set to ``true`` if you would like subsequent file dialogs to open
	 *     at whatever folder they were in when they close this one.
	 */
	std::string file_dialog(const std::vector< std::pair<std::string, std::string> > &filetypes, bool save);

	/**
	 * \brief Open a native file open dialog, which allows multiple selection.
	 *
	 * \param filetypes
	 *     Pairs of permissible formats with descriptions like
	 *     ``("png", "Portable Network Graphics")``.
	 *
	 * \param save
	 *     Set to ``true`` if you would like subsequent file dialogs to open
	 *     at whatever folder they were in when they close this one.
	 *
	 * \param multiple
	 *     Set to ``true`` if you would like to be able to select multiple
	 *     files at once. May not be simultaneously true with \p save.
	 */
	std::vector<std::string> file_dialog(const std::vector< std::pair<std::string, std::string> > &filetypes, bool save, bool multiple);
}

#endif	// _EASY3D_FILE_DIALOG_H_

