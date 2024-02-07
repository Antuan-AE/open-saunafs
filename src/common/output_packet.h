/*
   Copyright 2013-2014 EditShare
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

#include "protocol/packet.h"
#include "common/massert.h"

/// A struct used in servers for writing data to clients.
struct OutputPacket {
	OutputPacket(const PacketHeader& header) : bytesSent(0) {
		packet.reserve(PacketHeader::kSize + header.length);
		serialize(packet, header);
		packet.resize(PacketHeader::kSize + header.length);
	}

	OutputPacket(MessageBuffer message) : packet(std::move(message)), bytesSent(0) {}

	OutputPacket() : bytesSent(0) {}

	// Make it non-copyable (but movable) to avoid errors
	OutputPacket(OutputPacket&&) = default;
	OutputPacket(const OutputPacket&) = delete;
	OutputPacket& operator=(OutputPacket&&) = default;
	OutputPacket& operator=(const OutputPacket&) = delete;

	MessageBuffer packet;
	uint32_t bytesSent;
};
