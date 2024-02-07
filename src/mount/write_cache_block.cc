/*
   Copyright 2013-2019 Skytechnology sp. z o.o.
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

#include "common/platform.h"
#include "mount/write_cache_block.h"

#include <cstring>
#include <utility>

#include "common/massert.h"
#include "protocol/SFSCommunication.h"

WriteCacheBlock::WriteCacheBlock(uint32_t chunkIndex, uint32_t blockIndex, Type type)
		: chunkIndex(chunkIndex),
		  blockIndex(blockIndex),
		  from(0),
		  to(0),
		  type(type) {
	sassert(blockIndex < SFSBLOCKSINCHUNK);
	blockData = new uint8_t[SFSBLOCKSIZE];
}

WriteCacheBlock::WriteCacheBlock(WriteCacheBlock&& block) noexcept {
	blockData = block.blockData;
	block.blockData = nullptr;
	chunkIndex = block.chunkIndex;
	blockIndex = block.blockIndex;
	from = block.from;
	block.from = 0;
	to = block.to;
	block.to = 0;
	type = block.type;
}

WriteCacheBlock &WriteCacheBlock::operator=(WriteCacheBlock &&block) {
	std::swap(blockData, block.blockData);
	std::swap(chunkIndex, block.chunkIndex);
	std::swap(blockIndex, block.blockIndex);
	std::swap(from, block.from);
	std::swap(to, block.to);
	std::swap(type, block.type);
	return *this;
}

WriteCacheBlock::~WriteCacheBlock() {
	delete[] blockData;
}

bool WriteCacheBlock::expand(uint32_t from, uint32_t to, const uint8_t *buffer) {
	if (size() == 0) {
		this->from = from;
		this->to = to;
		memcpy(blockData + from, buffer, to - from);
		return true;
	}
	if (from > this->to || to < this->from) { // can't expand
		return false;
	}
	memcpy(blockData + from, buffer, to - from);
	if (from < this->from) {
		this->from = from;
	}
	if (to > this->to) {
		this->to = to;
	}
	return true;
}

uint64_t WriteCacheBlock::offsetInFile() const {
	return static_cast<uint64_t>(chunkIndex) * SFSCHUNKSIZE + offsetInChunk();
}

uint32_t WriteCacheBlock::offsetInChunk() const {
	return blockIndex * SFSBLOCKSIZE + from;
}

uint32_t WriteCacheBlock::size() const {
	return to - from;
}

const uint8_t* WriteCacheBlock::data() const {
	return blockData + from;
}

uint8_t* WriteCacheBlock::data() {
	return blockData + from;
}
