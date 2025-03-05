#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Simple command execution w echo" {
  run ./dsh <<EOF
echo hello
exit
EOF
  [[ "$output" =~ "hello" ]]
  [ "$status" -eq 0 ]
}

@test "Pipeline execution" {
  run ./dsh <<EOF
echo hello world | tr a-z A-Z
exit
EOF
  [[ "$output" =~ "HELLO WORLD" ]]
  [ "$status" -eq 0 ]
}

@test "Pipeline with three commands and arguments" {
  run ./dsh <<EOF
echo "first arg" | grep first | wc -w
exit
EOF
  [[ "$output" =~ "1" ]]
  [ "$status" -eq 0 ]
}

@test "Pipeline with four commands" {
  run ./dsh <<EOF
echo -e "c\nb\na" | sort | uniq | wc -l
exit
EOF
  [[ "$output" =~ "3" ]]
  [ "$status" -eq 0 ]
}

@test "Input redirection: < (Extra Credit)" {
  echo "input content" > input.txt
  run ./dsh <<EOF
cat < input.txt
exit
EOF
  [[ "$output" =~ "input content" ]]
  [ "$status" -eq 0 ]
  rm input.txt
}

@test "Append redirection: >> (Extra Credit)" {
  rm -f out.txt
  run ./dsh <<EOF
echo "first line" > out.txt
echo "second line" >> out.txt
cat out.txt
exit
EOF
  [[ "$output" =~ "first line" ]]
  [[ "$output" =~ "second line" ]]
  [ "$status" -eq 0 ]
  rm out.txt
}

@test "Output redirection: > (Extra Credit)" {
  run ./dsh <<EOF 
echo "hello, class" > out.txt
cat out.txt
exit
EOF
  [[ "$output" =~ "hello, class" ]]
  [ "$status" -eq 0 ]
}

@test "Exit command terminates the shell" {
  run ./dsh <<EOF
exit
EOF
  [[ "$output" =~ "dsh3>" ]]
  [ "$status" -eq 0 ]
}

@test "Change directory" {
  mkdir -p "$TMPDIR/testdir"
  run ./dsh <<EOF
cd "$TMPDIR/testdir"
pwd
exit
EOF
  [[ "$output" =~ "$TMPDIR/testdir" ]]
  [ "$status" -eq 0 ]
}

@test "Invalid command handling" {
  run ./dsh <<EOF
nosuchcommand
exit
EOF
  [[ "$output" =~ "execvp" || "$output" =~ "failed" || "$output" =~ "not found" ]]
  [ "$status" -eq 0 ]
}

@test "Quoted arguments handling" {
  run ./dsh <<EOF
echo "hello     world"
exit
EOF
  [[ "$output" =~ "hello     world" ]]
  [ "$status" -eq 0 ]
}

@test "Multiple spaces between arguments" {
  run ./dsh <<EOF
echo multiple    spaces
exit
EOF
  [[ "$output" =~ "multiple spaces" ]]
  [ "$status" -eq 0 ]
}

@test "Empty input produces prompt only" {
  run ./dsh <<EOF

exit
EOF
  [[ "$output" =~ "dsh3>" ]]
  [ "$status" -eq 0 ]
}

@test "Too many commands in pipeline" {
  run ./dsh <<EOF
cmd1 | cmd2 | cmd3 | cmd4 | cmd5 | cmd6 | cmd7 | cmd8 | cmd9
exit
EOF
  [[ "$output" =~ "error: piping limited to 8 commands" ]]
  [ "$status" -eq 0 ]
}

@test "Rapid successive commands execution" {
  run ./dsh <<EOF
echo one
echo two
echo three
exit
EOF
  [[ "$output" =~ "one" ]]
  [[ "$output" =~ "two" ]]
  [[ "$output" =~ "three" ]]
  [ "$status" -eq 0 ]
}

@test "Maximum argument limit test" {
  args=""
  for i in $(seq 1 10); do
    args="$args arg$i"
  done
  run ./dsh <<EOF
echo $args
exit
EOF
  [[ "$output" =~ "arg1" ]]
  [[ "$output" =~ "arg10" ]]
  [ "$status" -eq 0 ]
}