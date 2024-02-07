/*

   Copyright 2017 Skytechnology sp. z o.o.
   Copyright 2023 Leil Storage OÜ

   This file is part of SaunaFS.

   SaunaFS is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 3.

   SaunaFS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with SaunaFS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common/platform.h"

#include "common/serialization_macros.h"
#include "common/attributes.h"

namespace legacy {
SAUNAFS_DEFINE_SERIALIZABLE_CLASS(DirectoryEntry,
	uint32_t, inode,
	std::string, name,
	Attributes, attributes);
} // namespace legacy

SAUNAFS_DEFINE_SERIALIZABLE_CLASS(DirectoryEntry,
	uint64_t, index,
	uint64_t, next_index,
	uint32_t, inode,
	std::string, name,
	Attributes, attributes);
