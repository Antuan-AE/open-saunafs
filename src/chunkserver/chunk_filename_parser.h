/*
   Copyright 2013-2015 Skytechnology sp. z o.o.
   Copyright 2023      Leil Storage OÜ

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

#include <string>

#include "chunkserver-common/chunk_interface.h"
#include "common/chunk_part_type.h"
#include "common/parser.h"

class ChunkFilenameParser : public Parser {
public:
	enum Status {
		OK,
		ERROR_INVALID_FILENAME
	};

	ChunkFilenameParser(const std::string& filename);
	Status parse();
	ChunkFormat chunkFormat() const;
	ChunkPartType chunkType() const;
	uint32_t chunkVersion() const;
	uint64_t chunkId() const;

private:
	static const size_t kChunkVersionStringSize = 8;
	static const size_t kChunkIdStringSize = 16;

	ChunkFormat chunkFormat_;
	std::string chunkName_;
	ChunkPartType chunkType_;
	uint32_t chunkVersion_;
	uint64_t chunkId_;

	Status parseXorChunkType();
	Status parseECChunkType();
	Status parseChunkType();
};
