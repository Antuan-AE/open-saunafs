timeout_set 2 hours
assert_program_installed git
assert_program_installed cmake
assert_program_installed rsync

MINIMUM_PARALLEL_JOBS=5
MAXIMUM_PARALLEL_JOBS=16
PARALLEL_JOBS=$(get_nproc_clamped_between ${MINIMUM_PARALLEL_JOBS} ${MAXIMUM_PARALLEL_JOBS})

test_worker() {
	export MESSAGE="Testing directory $1"
	cd "$1"
	assertlocal_success git clone https://github.com/saunafs/saunafs.git
	mkdir saunafs/build
	cd saunafs/build
	assertlocal_success cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install
	make -j${PARALLEL_JOBS} install
	cd ../..
	assertlocal_success rsync -a saunafs/ copy_saunafs
	find saunafs -type f | while read file; do
		expect_files_equal "$file" "copy_$file"
	done
}

CHUNKSERVERS=3 \
	MOUNTS=1 \
	MOUNT_EXTRA_CONFIG="sfscachemode=NEVER" \
	setup_local_empty_saunafs info

cd "${info[mount0]}"
for goal in 1 2 3 xor2; do
	mkdir "goal_$goal"
	saunafs setgoal "$goal" "goal_$goal"
	test_worker "goal_$goal" &
done
wait
