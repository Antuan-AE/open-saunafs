timeout_set 3 minutes

CHUNKSERVERS=3 \
	USE_RAMDISK=YES \
	setup_local_empty_saunafs info

file_size_mb=800
cd "${info[mount0]}"

for goal in 1 2 xor2 xor3; do
	test_filename=speed_test_file_${goal}
	touch "$test_filename"
	saunafs setgoal $goal "$test_filename"

	# write file and measure time
	drop_caches
	time_file=$TEMP_DIR/$(unique_file)
		/usr/bin/time -o "$time_file" -f %e dd \
		if=/dev/zero \
		of="$test_filename" \
		bs=128K \
		count=$((file_size_mb * 8)) \
		conv=fsync
	write_time=$(cat "$time_file")
	write_speed=$(echo "scale=3;${file_size_mb}/${write_time}" | bc)

	# read file and measure time
	drop_caches
	/usr/bin/time -o "$time_file" -f %e dd \
		if="$test_filename" \
		of=/dev/null \
		bs=64K
	read_time=$(cat "$time_file")
	read_speed=$(echo "scale=3;${file_size_mb}/${read_time}" | bc)

	# write test results to a csv file
	echo -e "Goal ${goal}\n${write_speed}" > "${TEMP_DIR}/write_${goal}.csv"
	echo -e "Goal ${goal}\n${read_speed}" > "${TEMP_DIR}/read_$goal.csv"

	# wait for chunkservers to close chunks and remove them immediately
	sleep 10
	find_all_chunks | xargs rm -f
done

# Create a file with all the results in the test's output dir
paste -d, $TEMP_DIR/write_*.csv | tee "${TEST_OUTPUT_DIR}/ramdisk_write_speed_results.csv"
paste -d, $TEMP_DIR/read_*.csv | tee "${TEST_OUTPUT_DIR}/ramdisk_read_speed_results.csv"
