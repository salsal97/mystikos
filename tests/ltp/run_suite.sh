function run_tests() {
FILE=$1

# rm *.output*

while read test; do
  echo "$test"
  OUTPUT=$(2>&1 make one TEST=$test)
  echo $OUTPUT >> op_$FILE.output
  HAS_UNHANDLED_SYSCALL=$(2>&1 make one TEST=$test | grep "unhandled")
  if [ -z "$HAS_UNHANDLED_SYSCALL" ]
  then
    # No unhandled syscall
    echo $test >> op_other_errors.output
  else
    echo "$test: $HAS_UNHANDLED_SYSCALL" >> op_unhandled_syscalls.output
  fi
done <$FILE

cat op_$FILE.output | grep "unhandled syscall:" > op_unhandled_syscalls.output.crosscheck

}

run_tests ltp1.txt
run_tests ltp2.txt