/*

   Copyright 2015 Skytechnology sp. z o.o.
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

#include "common/platform.h"

#include "master/locks.h"

#include <gtest/gtest.h>

inline bool add(LockRanges &ranges, LockRange &range) {
	bool ok;

	ok = ranges.fits(range);
	if (!ok) {
		return false;
	}
	ranges.insert(range);

	return true;
}

inline bool add(LockRanges &ranges, LockRange::Type type, off_t start, off_t end, uint64_t owner) {
	LockRange range(type, start, end, {owner, 0, 0, 0});

	return add(ranges, range);
}

inline bool lock_shared(LockRanges &ranges, off_t start, off_t end, int owner) {
	return add(ranges,  LockRange::Type::kShared, start, end, owner);
}

inline bool lock_exclusive(LockRanges &ranges, off_t start, off_t end, int owner) {
	return add(ranges,  LockRange::Type::kExclusive, start, end, owner);
}

inline bool unlock(LockRanges &ranges, off_t start, off_t end, int owner) {
	return add(ranges,  LockRange::Type::kUnlock, start, end, owner);
}

TEST(LocksTest, ExclusiveOverwrite) {
	LockRanges ranges;

	// Create a lock on range [10, 30)
	EXPECT_TRUE(lock_exclusive(ranges, 10, 30, 1));
	// Try to lock [20, 40) as other user and fail
	EXPECT_FALSE(lock_exclusive(ranges, 20, 40, 2));
	// Overwrite range [10, 30) with [10, 40)
	EXPECT_TRUE(lock_exclusive(ranges, 20, 40, 1));
	// Remove lock from [20, 40), overwritten range from above should not reappear...
	EXPECT_TRUE(unlock(ranges, 20, 40, 1));
	// ...so it is possible to lock [20, 40) as the other user now
	EXPECT_TRUE(lock_exclusive(ranges, 20, 40, 2));
}

TEST(LocksTest, SameOwner) {
	LockRanges ranges;

	// Ranges created by the same owner should overwrite each other,
	// even if it means splitting, merging and deleting some of them.
	EXPECT_TRUE(lock_exclusive(ranges, 0, 100, 1));
	EXPECT_TRUE(lock_shared(ranges, 1, 3, 1));
	EXPECT_TRUE(lock_shared(ranges, 8, 9, 1));
	EXPECT_TRUE(lock_shared(ranges, 10, 15, 1));
	EXPECT_TRUE(lock_shared(ranges, 5, 7, 1));
	EXPECT_TRUE(lock_exclusive(ranges, 2, 12, 1));
	EXPECT_TRUE(lock_shared(ranges, 4, 5, 1));
	EXPECT_TRUE(lock_shared(ranges, 40, 50, 1));
	EXPECT_TRUE(lock_shared(ranges, 45, 55, 1));

	std::vector<LockRange> out = {
		{LockRange::Type::kExclusive, 0, 1, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 1, 2, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 2, 4, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 4, 5, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 5, 12, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 12, 15, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 15, 40, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 40, 55, {2, 0, 0, 0}},
		{LockRange::Type::kExclusive, 55, 100, {2, 0, 0, 0}},
	};

	for (unsigned i = 0; i < out.size(); ++i) {
		EXPECT_FALSE(add(ranges, out[i]));
	}

}

TEST(LocksTest, OverlappingReads) {
	LockRanges ranges;

	// Shared ranges should not be exclusive (yes, really)
	EXPECT_TRUE(lock_shared(ranges, 0, 100, 1));
	EXPECT_TRUE(lock_shared(ranges, 1, 3, 2));
	EXPECT_TRUE(lock_shared(ranges, 8, 9, 3));
	EXPECT_TRUE(lock_shared(ranges, 10, 15, 4));
	EXPECT_TRUE(lock_shared(ranges, 5, 7, 5));
	EXPECT_TRUE(lock_shared(ranges, 2, 12, 6));
	EXPECT_TRUE(lock_shared(ranges, 4, 5, 7));
	EXPECT_TRUE(lock_shared(ranges, 40, 50, 8));
	EXPECT_TRUE(lock_shared(ranges, 45, 55, 9));
	// Exclusive range should fail if another read range (besides his one) is held
	EXPECT_FALSE(lock_exclusive(ranges, 1, 2, 1));
}

TEST(LocksTest, Removes) {
	LockRanges ranges;

	// Create range
	EXPECT_TRUE(lock_exclusive(ranges, 0, 100, 1));
	// Create holes in range
	EXPECT_TRUE(unlock(ranges, 10, 20, 1));
	EXPECT_TRUE(unlock(ranges, 40, 50, 1));
	EXPECT_TRUE(unlock(ranges, 70, 90, 1));
	// Removing as a different user should have no effect
	EXPECT_TRUE(unlock(ranges, 90, 100, 2));

	EXPECT_TRUE(lock_exclusive(ranges, 0, 90, 1));

}

TEST(LocksTest, AdjacentRanges) {
	LockRanges ranges;

	// Adjacent ranges of the same user should be merged
	EXPECT_TRUE(lock_shared(ranges, 10, 20, 1));
	EXPECT_EQ(1U, ranges.size());
	EXPECT_TRUE(lock_shared(ranges, 40, 50, 1));
	EXPECT_EQ(2U, ranges.size());
	EXPECT_TRUE(lock_shared(ranges, 25, 30, 1));
	EXPECT_EQ(3U, ranges.size());
	EXPECT_TRUE(lock_shared(ranges, 20, 25, 1));
	EXPECT_EQ(2U, ranges.size());
	EXPECT_TRUE(lock_shared(ranges, 30, 40, 1));
	EXPECT_EQ(1U, ranges.size());
	EXPECT_TRUE(lock_exclusive(ranges, 0, 100, 1));
}

TEST(LocksTest, StackedRead) {
	LockRanges ranges;

	// Create a stack of read locks with height 3
	EXPECT_TRUE(lock_shared(ranges, 10, 20, 1));
	EXPECT_TRUE(lock_shared(ranges, 10, 20, 2));
	EXPECT_TRUE(lock_shared(ranges, 10, 20, 3));
	// Write lock is unavailable
	EXPECT_FALSE(lock_exclusive(ranges, 10, 20, 1));
	// Remove top read lock from the stack
	EXPECT_TRUE(unlock(ranges, 10, 20, 3));
	// Write lock is still unavailable
	EXPECT_FALSE(lock_exclusive(ranges, 10, 20, 1));
	// Remove next read lock from the stack
	EXPECT_TRUE(unlock(ranges, 10, 20, 2));
	// Write lock should still be unavailable...
	EXPECT_FALSE(lock_exclusive(ranges, 10, 20, 2));
	// ...except for the owner of the existing read lock
	EXPECT_TRUE(lock_exclusive(ranges, 10, 20, 1));
}

TEST(LocksTest, ReadHolePunching) {
	LockRanges ranges;

	// Create exclusive lock
	EXPECT_TRUE(lock_exclusive(ranges, 0, 100, 1));
	// Other locks are unavailable for user 2
	EXPECT_FALSE(lock_shared(ranges, 30, 60, 2));
	EXPECT_FALSE(lock_exclusive(ranges, 30, 60, 2));
	// Punch a read hole in exclusive lock
	EXPECT_TRUE(lock_shared(ranges, 40, 50, 1));
	// Hole is to small, locking is still not possible for user 2
	EXPECT_FALSE(lock_shared(ranges, 30, 60, 2));
	EXPECT_FALSE(lock_exclusive(ranges, 30, 60, 2));
	// Punch a bigger read hole in exclusive lock
	EXPECT_TRUE(lock_shared(ranges, 30, 60, 1));
	// It is now possible for user 2 to place a shared lock
	EXPECT_TRUE(lock_shared(ranges, 30, 60, 2));
	EXPECT_FALSE(lock_exclusive(ranges, 30, 60, 2));
}

TEST(LocksTest, PartialUnlock) {
	LockRanges ranges;

	// Create 2 shared locks
	 EXPECT_TRUE(lock_shared(ranges, 1, 3, 1));
	 EXPECT_TRUE(lock_shared(ranges, 1, 3, 2));
	 // Exclusive lock should fail
	 EXPECT_FALSE(lock_exclusive(ranges, 1, 2, 1));
	 // Unlock 1 of the shared locks
	 EXPECT_TRUE(unlock(ranges, 1, 2, 1));
	 // Exclusive lock should still fail
	 EXPECT_FALSE(lock_exclusive(ranges, 1, 2, 1));
	 // Unlock the last shared lock
	 EXPECT_TRUE(unlock(ranges, 1, 2, 2));
	 // Exclusive lock should now succeed
	 EXPECT_TRUE(lock_exclusive(ranges, 1, 2, 1));
}

TEST(LocksTest, StressTest) {
	LockRanges ranges;

	EXPECT_TRUE(lock_exclusive(ranges, 0, 1, 7));

	for(int j = 0; j < 100; j++) {
		int i = j % 3;
		int a = i;
		int b = (i + 1) % 3;
		int c = (i + 2) % 3;

		EXPECT_TRUE(lock_exclusive(ranges, b, b + 1, 7));
		EXPECT_TRUE(unlock(ranges, a, a + 1, 7));
		EXPECT_TRUE(lock_exclusive(ranges, c, c + 1, 7));
	}

	EXPECT_TRUE(lock_exclusive(ranges, 0, 1, 6));

	EXPECT_TRUE(unlock(ranges, 1, 2, 7));
	EXPECT_FALSE(lock_exclusive(ranges, 0, 1, 7));

	EXPECT_TRUE(lock_exclusive(ranges, 1, 2, 6));
	EXPECT_TRUE(unlock(ranges, 0, 1, 6));
	EXPECT_TRUE(lock_exclusive(ranges, 0, 1, 7));
}

/*
 * FileLocks class tests
 */

static std::vector<FileLocks::Owner> owners = {
	{0, 1, 2, 3},
	{1, 2, 3, 4},
	{2, 3, 4, 5},
	{3, 4, 5, 6},
	{4, 5, 6, 7},
	{5, 6, 7, 8},
};

TEST(LocksTest, Probe) {
	FileLocks locks;
	const LockRange *collision;

	collision = locks.findCollision(0, LockRange::Type::kExclusive, 2, 5, owners[0]);
	EXPECT_EQ(nullptr, collision);

	EXPECT_TRUE(locks.sharedLock(0, 0, 10, owners[1]));
	collision = locks.findCollision(0, LockRange::Type::kExclusive, 2, 5, owners[0]);
	EXPECT_EQ(0U, collision->start);
	EXPECT_EQ(10U, collision->end);
	EXPECT_EQ(collision->type, LockRange::Type::kShared);

	EXPECT_TRUE(locks.unlock(0, 0, 10, owners[1]));
	collision = locks.findCollision(0, LockRange::Type::kExclusive, 2, 5, owners[0]);
	EXPECT_EQ(nullptr, collision);

	locks.clear();
}

TEST(LocksTest, Unqueue) {
	FileLocks locks;
	FileLocks::LockQueue queue;

	EXPECT_TRUE(locks.exclusiveLock(0, 0, 10, owners[1]));
	// 3 locks will be enqueued
	EXPECT_FALSE(locks.exclusiveLock(0, 0, 3, owners[2]));
	EXPECT_FALSE(locks.exclusiveLock(0, 3, 5, owners[3]));
	EXPECT_FALSE(locks.exclusiveLock(0, 5, 10, owners[4]));

	// owner[3]'s lock will be removed from queue
	locks.removePending(0,
		[](const FileLocks::Lock &lock){
			auto owner = lock.owner();
			return owner.owner == owners[3].owner
				&& owner.sessionid == owners[3].sessionid
				&& owner.reqid == owners[3].reqid;
		});

	// Only 2 locks should be in candidate queue,
	// since one of them was unqueued
	EXPECT_TRUE(locks.unlock(0, 0, 10, owners[1]));
	locks.gatherCandidates(0, 0, 10, queue);
	EXPECT_EQ(2U, queue.size());
	for (auto queued : queue) {
		EXPECT_TRUE(locks.apply(0, queued));
	}
	queue.clear();

	// Range [3, 5) should be available for locking, since lock 3 was removePendingd
	EXPECT_TRUE(locks.sharedLock(0, 3, 5, owners[1]));

	// Read lock should be enqueued because of applied write locks
	EXPECT_FALSE(locks.sharedLock(0, 3, 6, owners[5]));

	// Read lock will be unqueued
	locks.removePending(0,
		[](const FileLocks::Lock &lock){
			auto owner = lock.owner();
			return owner.owner == owners[5].owner
				&& owner.sessionid == owners[5].sessionid
				&& owner.reqid == owners[5].reqid;
		});

	// Write lock is partially removed to allow potential read locks
	EXPECT_TRUE(locks.unlock(0, 5, 6, owners[4]));

	// But the read lock was unqueued, so no candidates should exist in queue
	locks.gatherCandidates(0, 0, 10, queue);
	EXPECT_EQ(0U, queue.size());

	locks.clear();
}
