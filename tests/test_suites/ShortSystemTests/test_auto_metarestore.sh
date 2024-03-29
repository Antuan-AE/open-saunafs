timeout_set 3 minutes
USE_RAMDISK=YES \
	setup_local_empty_saunafs info
cat $TEMP_DIR/saunafs/etc/sfsmetalogger.cfg
saunafs_metalogger_daemon start

# Create 100 files and save 5 metadata files containing 20, 40, 60, 80 and 100 of them. Remove all
# changelogs after 40 files, so the filesystem can't be recovered from metadata.1 and metadata.2
for i in {1..5}; do
	touch "${info[mount0]}/file_$i."{1..20}
	saunafs_master_daemon stop
	cp "${info[master_data_path]}/metadata.sfs" "$TEMP_DIR/metadata.$i"
	if (( $i == 2 )); then
		saunafs_metalogger_daemon stop
		rm "${info[master_data_path]}"/*changelog*
		saunafs_metalogger_daemon start
	fi
	saunafs_master_daemon start
done
saunafs_master_daemon kill
saunafs_metalogger_daemon kill

# Function takes three metadata files as arguments and tries to recover the filesystem
verify_recovery() {
	declare -A files=([metadata.sfs]="$1" [metadata.sfs.1]="$2" [metadata_ml.sfs]="$3")
	rm -f "${info[master_data_path]}"/metadata*
	local filelist=""
	for file in "${!files[@]}"; do
		if [[ ${files[$file]} ]]; then
			cp -a "${files[$file]}" "${info[master_data_path]}/$file"
			filelist="$filelist $file=$(basename ${files[$file]})"
		fi
	done
	(
		export MESSAGE="$MESSAGE (from$filelist)"
		echo "$MESSAGE"
		assertlocal_success sfsmetarestore -d "${info[master_data_path]}" -a
		assertlocal_success saunafs_master_daemon start
		expect_equals 100 $(ls "${info[mount0]}" | wc -l)
		expect_success saunafs_master_daemon kill
	) || true
}

MESSAGE="Veryfing recovery when nothing is needed to do"
verify_recovery "$TEMP_DIR/metadata.5" "$TEMP_DIR/metadata.4" ""

MESSAGE="Veryfing recovery only from the metalogger's file"
verify_recovery "" "" "$TEMP_DIR/metadata.3"
verify_recovery "" "" "$TEMP_DIR/metadata.5"

MESSAGE="Veryfing recovery only from the master's file"
verify_recovery "" "$TEMP_DIR/metadata.3" ""
verify_recovery "" "$TEMP_DIR/metadata.5" ""

MESSAGE="Veryfing recovery from a new master's backup and an outdated metalogger's backup"
verify_recovery "" "$TEMP_DIR/metadata.3" "$TEMP_DIR/metadata.1"
verify_recovery "" "$TEMP_DIR/metadata.5" "$TEMP_DIR/metadata.1"
verify_recovery "$TEMP_DIR/metadata.3" "" "$TEMP_DIR/metadata.1"
verify_recovery "$TEMP_DIR/metadata.5" "" "$TEMP_DIR/metadata.1"

MESSAGE="Veryfing recovery from new metalogger's backup and an outdated master's backup"
verify_recovery "" "$TEMP_DIR/metadata.1" "$TEMP_DIR/metadata.3"
verify_recovery "" "$TEMP_DIR/metadata.1" "$TEMP_DIR/metadata.5"
verify_recovery "$TEMP_DIR/metadata.1" "" "$TEMP_DIR/metadata.4"
verify_recovery "$TEMP_DIR/metadata.1" "" "$TEMP_DIR/metadata.5"

MESSAGE="Veryfing recovery when metadata.sfs is older than a backup"
verify_recovery "$TEMP_DIR/metadata.1" "$TEMP_DIR/metadata.4" ""
verify_recovery "$TEMP_DIR/metadata.1" "$TEMP_DIR/metadata.5" ""

MESSAGE="Veryfing recovery from some metalogger's backup and a corrupted master's backup"
dd of="$TEMP_DIR/metadata.4" conv=notrunc <<< paczek
echo 2paczki >> "$TEMP_DIR/metadata.5"
verify_recovery "$TEMP_DIR/metadata.5" "" "$TEMP_DIR/metadata.3"
verify_recovery "$TEMP_DIR/metadata.5" "$TEMP_DIR/metadata.4" "$TEMP_DIR/metadata.3"
