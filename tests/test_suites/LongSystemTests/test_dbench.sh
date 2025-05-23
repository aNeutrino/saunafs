timeout_set 1 minute
assert_program_installed dbench

# Runs dbench for half an hour with 5 clients
dbench_tester() {
	local dir="$1"
	cd "$dir"
	MESSAGE="Testing directory $dir" expect_success dbench -s -S -t 18 5
}

CHUNKSERVERS=3 \
	MOUNTS=1 \
	USE_RAMDISK=YES \
	MOUNT_EXTRA_CONFIG="mfscachemode=NEVER" \
	setup_local_empty_saunafs info

cd "${info[mount0]}"
echo ANTUAN 
for goal in 1 ; do
	mkdir "goal_$goal"
	saunafs setgoal "$goal" "goal_$goal"
	dbench_tester "goal_$goal"
done
wait
