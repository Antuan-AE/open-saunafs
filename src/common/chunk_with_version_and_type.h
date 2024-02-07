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

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <tuple>

#include "common/chunk_part_type.h"
#include "common/serialization_macros.h"
#include "common/slice_traits.h"

namespace legacy {

struct ChunkWithVersionAndType {
	uint64_t id;
	uint32_t version;
	ChunkPartType type;

	ChunkWithVersionAndType() : id(0), version(0), type(0) {}

	ChunkWithVersionAndType(uint64_t id, uint32_t version, ChunkPartType type)
			: id(id),
			  version(version),
			  type(type) {
	}

	std::string toString() const {
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(16) << id << '_';
		ss << std::setw(8) << version;
		ss << " (" << type.toString() << ")";
		return ss.str();
	}

	bool operator<(const ChunkWithVersionAndType& other) const {
		return std::make_tuple(id, version, type)
			< std::make_tuple(other.id, other.version, other.type);
	}
	bool operator==(const ChunkWithVersionAndType& other) const {
		return id == other.id && version == other.version && type == other.type;
	}

	SAUNAFS_DEFINE_SERIALIZE_METHODS(id, version, type);
};

} // legacy

struct ChunkWithVersionAndType {
	uint64_t id;
	uint32_t version;
	ChunkPartType type;

	ChunkWithVersionAndType() : id(0), version(0), type(slice_traits::standard::ChunkPartType()) {}

	ChunkWithVersionAndType(uint64_t id, uint32_t version, ChunkPartType type)
			: id(id),
			  version(version),
			  type(type) {
	}

	ChunkWithVersionAndType(const legacy::ChunkWithVersionAndType &other)
	: id(other.id), version(other.version), type(other.type) {}

	std::string toString() const {
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(16) << id << '_';
		ss << std::setw(8) << version;
		ss << " (" << type.toString() << ")";
		return ss.str();
	}

	bool operator<(const ChunkWithVersionAndType& other) const {
		return std::make_tuple(id, version, type)
			< std::make_tuple(other.id, other.version, other.type);
	}
	bool operator==(const ChunkWithVersionAndType& other) const {
		return id == other.id && version == other.version && type == other.type;
	}

	SAUNAFS_DEFINE_SERIALIZE_METHODS(id, version, type);
};
