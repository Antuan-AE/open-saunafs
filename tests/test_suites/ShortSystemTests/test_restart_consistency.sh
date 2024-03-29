timeout_set 10 minutes
assert_program_installed setfacl getfacl python3
touch "$TEMP_DIR/f"
MESSAGE="Testing ACL support in $TEMP_DIR/" assert_success setfacl -m group:fuse:rw "$TEMP_DIR/f"

CHUNKSERVERS=1 \
	USE_RAMDISK=YES \
	SFSEXPORTS_EXTRA_OPTIONS=nomasterpermcheck,ignoregid \
	SAFS_MOUNT_COMMAND=sfsmount \
	MOUNT_EXTRA_CONFIG="sfscachemode=NEVER" \
	setup_local_empty_saunafs info

saudir="${info[mount0]}/subdir"
tmpdir="$TEMP_DIR/subdir"
mkdir -p "$saudir" "$tmpdir"
chmod 770 "$saudir" "$tmpdir"

# Do the same things in two trees (saudir and tmpdir) and compare permissions after each command
do_iteration() {
	iteration=$1

	while read command; do
		user_1=$((iteration % 4))
		user_2=$(((1 + iteration) % 4))
		# Strip the trailing comment, replace variables $file, $u1, $u2
		command=$(sed -e 's/ *#.*//;s/$u1/'$user_1'/;s/$u2/'$user_2'/;s/$file/file_'$iteration'/' <<< "$command")
		[[ $command ]] || continue
		echo "$command"
		export MESSAGE="Executing '$command' in both directory trees"
		( cd "$tmpdir" ; assertlocal_success eval "$command" )
		( cd "$saudir" ; assertlocal_success eval "$command" )

		if [[ $iteration -gt 2 ]]; then
			saunafs_master_daemon restart
			saunafs_wait_for_all_ready_chunkservers
			assert_eventually "saunafs_master_daemon isalive"
		fi

		cd "$saudir"
		assert_equals "$(find $saudir -printf '%P\n' 2>/dev/null | sort)" "$(find $tmpdir -printf '%P\n' 2>/dev/null | sort)"

		find . 2>/dev/null | while read f; do
			for user in saunafstest saunafstest_{0..3}; do
				export MESSAGE="Veryfing permissions on $f as $user after '$command'"
				expected_perm=$(describe_permissions "$user" "$tmpdir/$f")
				actual_perm=$(describe_permissions "$user" "$saudir/$f")
				assert_equals "$expected_perm" "$actual_perm"
			done
		done
	done <<-'END'
		touch $file
		chmod 600 $file
		setfacl -m user:saunafstest_$u1:rwx $file
		setfacl -m user:saunafstest_$u2:rw- $file
		setfacl -m group:saunafstest:r-- $file
	END
}

for iteration in {1..8}; do
	do_iteration $iteration
done
