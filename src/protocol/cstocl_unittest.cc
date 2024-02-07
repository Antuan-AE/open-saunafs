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

#include "common/platform.h"
#include "protocol/cstocl.h"

#include <gtest/gtest.h>

#include "common/crc.h"
#include "unittests/inout_pair.h"
#include "unittests/packet.h"

TEST(CltocsCommunicationTests, ReadData) {
	SAUNAFS_DEFINE_INOUT_PAIR(uint64_t, chunkId, 0x0123456789ABCDEF, 0);
	SAUNAFS_DEFINE_INOUT_PAIR(uint32_t, readOffset, 2 * SFSBLOCKSIZE, 0);
	SAUNAFS_DEFINE_INOUT_PAIR(uint32_t, readSize, SFSBLOCKSIZE, 0);
	SAUNAFS_DEFINE_INOUT_PAIR(uint32_t, crc, 0x89ABCDEF, 0);

	std::vector<uint8_t> buffer;
	ASSERT_NO_THROW(cstocl::readData::serializePrefix(buffer,
			chunkIdIn, readOffsetIn, readSizeIn));
	uint32_t prefixSize = buffer.size();
	buffer.resize(prefixSize + serializedSize(crcIn) + SFSBLOCKSIZE);
	uint8_t* ptr = buffer.data() + prefixSize;
	ASSERT_NO_THROW(serialize(&ptr, crcIn));

	verifyHeader(buffer, SAU_CSTOCL_READ_DATA);
	removeHeaderInPlace(buffer);
	ASSERT_NO_THROW(cstocl::readData::deserializePrefix(buffer,
			chunkIdOut, readOffsetOut, readSizeOut, crcOut));

	SAUNAFS_VERIFY_INOUT_PAIR(chunkId);
	SAUNAFS_VERIFY_INOUT_PAIR(readOffset);
	SAUNAFS_VERIFY_INOUT_PAIR(readSize);
	SAUNAFS_VERIFY_INOUT_PAIR(crc);
}

TEST(CltocsCommunicationTests, ReadStatus) {
	SAUNAFS_DEFINE_INOUT_PAIR(uint64_t, chunkId,  0x0123456789ABCDEF, 0);
	SAUNAFS_DEFINE_INOUT_PAIR(uint8_t,  status,   12,                 0);

	std::vector<uint8_t> buffer;
	ASSERT_NO_THROW(cstocl::readStatus::serialize(buffer, chunkIdIn, statusIn));

	verifyHeader(buffer, SAU_CSTOCL_READ_STATUS);
	removeHeaderInPlace(buffer);
	ASSERT_NO_THROW(cstocl::readStatus::deserialize(buffer, chunkIdOut, statusOut));

	SAUNAFS_VERIFY_INOUT_PAIR(chunkId);
	SAUNAFS_VERIFY_INOUT_PAIR(status);
}

TEST(CltocsCommunicationTests, WriteStatus) {
	SAUNAFS_DEFINE_INOUT_PAIR(uint64_t, chunkId,  0x0123456789ABCDEF, 0);
	SAUNAFS_DEFINE_INOUT_PAIR(uint32_t, writeId,  0x12345678,         0);
	SAUNAFS_DEFINE_INOUT_PAIR(uint8_t,  status,   12,                 0);

	std::vector<uint8_t> buffer;
	ASSERT_NO_THROW(cstocl::writeStatus::serialize(buffer, chunkIdIn, writeIdIn, statusIn));

	verifyHeader(buffer, SAU_CSTOCL_WRITE_STATUS);
	removeHeaderInPlace(buffer);
	ASSERT_NO_THROW(cstocl::writeStatus::deserialize(buffer, chunkIdOut, writeIdOut, statusOut));

	SAUNAFS_VERIFY_INOUT_PAIR(chunkId);
	SAUNAFS_VERIFY_INOUT_PAIR(writeId);
	SAUNAFS_VERIFY_INOUT_PAIR(status);
}
