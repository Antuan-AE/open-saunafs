timeout_set 10 minutes

CHUNKSERVERS=8 \
	MOUNT_EXTRA_CONFIG="sfscachemode=NEVER" \
	CHUNKSERVER_EXTRA_CONFIG="READ_AHEAD_KB = 1024|MAX_READ_BEHIND_KB = 2048"
	setup_local_empty_saunafs info

MINIMUM_PARALLEL_JOBS=4
MAXIMUM_PARALLEL_JOBS=16
PARALLEL_JOBS=$(get_nproc_clamped_between ${MINIMUM_PARALLEL_JOBS} ${MAXIMUM_PARALLEL_JOBS})

cd ${info[mount0]}

mkdir work
saunafs setgoal ec43 work

cd work

git clone "https://github.com/saunafs/saunafs.git"

cd saunafs

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j${PARALLEL_JOBS}
