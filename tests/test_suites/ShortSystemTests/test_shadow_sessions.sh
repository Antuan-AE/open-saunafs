timeout_set '160 seconds'

# Test if sessions are properly supported after promoting the shadow server.
MOUNTS=2 \
	MASTERSERVERS=2 \
	CHUNKSERVERS=1 \
	USE_RAMDISK=YES \
	MOUNT_EXTRA_CONFIG="sfscachemode=NEVER" \
	MOUNT_0_EXTRA_EXPORTS="ro,allcanchangequota" \
	MOUNT_1_EXTRA_EXPORTS="rw,alldirs,allcanchangequota,maxtrashtime=1234567,mapall=saunafstest_6:saunafstest_4" \
	setup_local_empty_saunafs info

mkdir "${info[mount1]}/subdir"

echo 'sfssubfolder=/subdir' >>"${info[mount1_cfg]}"
saunafs_mount_unmount 1
saunafs_mount_start 1

saunafs_master_n 1 start

cd "${info[mount1]}"
for generator in $(metadata_get_all_generators |grep -v metadata_generate_uids_gids); do
	eval "$generator"
done
metadata_validate_files

# Check if using removed files works as expected:
echo "ala ma kota" > removed_file
saunafs settrashtime 0 removed_file
exec 11<> removed_file
rm removed_file
echo -n "u huhu" >&11
assert_equals "u huhu kota" "$(cat /proc/$$/fd/11)"
cd

mount1meta=$(metadata_print "${info[mount1]}")
mount0meta=$(metadata_print "${info[mount0]}")

sleep 3
saunafs_master_daemon kill
saunafs_make_conf_for_master 1
saunafs_master_daemon reload
saunafs_wait_for_all_ready_chunkservers

# check restored filesystem
assert_no_diff "$mount0meta" "$(metadata_print "${info[mount0]}")"
assert_failure touch "${info[mount0]}"/newfile
cd "${info[mount1]}"
assert_no_diff "$mount1meta" "$(metadata_print)"
assert_success touch newfile
touch nowaythiswilleverwork
assert_failure saunafs settrashtime 12345678 nowaythiswilleverwork

# Check if using removed files works as expected after promotion:
echo -n " prrrrrr" >&11
assert_equals "u huhu prrrrrr" "$(cat /proc/$$/fd/11)"
exec 11>&-      # close the descriptor to allow clean umount

metadata_validate_files
