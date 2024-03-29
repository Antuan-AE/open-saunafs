USE_RAMDISK=YES \
	MASTER_EXTRA_CONFIG="METADATA_DUMP_PERIOD_SECONDS = 0" \
	setup_local_empty_saunafs info

# Do some changes and stop the master server in the "quick" mode
FILE_SIZE=1K assert_success file-generate "${info[mount0]}"/file_{1..5}
assert_success saunafs_admin_master stop-master-without-saving-metadata
assert_eventually "! saunafs_master_daemon isalive"

# Make sure that it can't start normally, but can start
assert_failure saunafs_master_daemon start
assert_success saunafs_master_daemon start -o auto-recovery
saunafs_wait_for_all_ready_chunkservers
assert_success file-validate "${info[mount0]}"/file_{1..5}

# Do some changes and stop the master server by killing it
FILE_SIZE=1K assert_success file-generate "${info[mount0]}"/file_{6..10}
assert_success saunafs_master_daemon kill
assert_failure "saunafs_master_daemon isalive"

# Make sure that it can't start normally, but can start by adding AUTO_RECOVERY=1
assert_failure saunafs_master_daemon start
echo "AUTO_RECOVERY = 1" >> "${info[master_cfg]}"
assert_success saunafs_master_daemon start
saunafs_wait_for_all_ready_chunkservers
assert_success file-validate "${info[mount0]}"/file_{1..10}
