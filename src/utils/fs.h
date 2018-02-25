/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#ifndef G_UTILS_FS_H
#define G_UTILS_FS_H


#include <string>


bool gu_fileExists(const std::string& path);
bool gu_dirExists(const std::string& path);
bool gu_isDir(const std::string& path);

/* isRootDir
Tells whether 's' is '/' on Unix or '[X]:\' on Windows. */

bool gu_isRootDir(const std::string& s);

bool gu_isProject(const std::string& path);
bool gu_mkdir(const std::string& path);
std::string gu_getCurrentPath();
std::string gu_getHomePath();

/* gu_basename
/path/to/file.txt -> file.txt */

std::string gu_basename(const std::string& s);

/* gu_dirname
/path/to/file.txt -> /path/to */

std::string gu_dirname(const std::string& s);

/* gu_getExt
/path/to/file.txt -> txt */

std::string gu_getExt(const std::string& s);

/* gu_stripExt
/path/to/file.txt -> /path/to/file */

std::string gu_stripExt(const std::string& s);

std::string gu_stripFileUrl(const std::string& s);

/* gu_getUpDir
Returns the upper directory:
/path/to/my/directory -> /path/to/my/ */

std::string gu_getUpDir(const std::string& s);


#endif
