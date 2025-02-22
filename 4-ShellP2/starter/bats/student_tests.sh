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

@test "Exit command" {
  run ./dsh <<EOF
exit
echo "Should not appear"
EOF
  [[ "$output" =~ "cmd loop returned 0" ]]
  [ "$status" -eq 0 ]
}

@test "Change directory" {
  mkdir -p /tmp/dsh_test_dir
  run ./dsh <<EOF
cd /tmp/dsh_test_dir
pwd
EOF

  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected="/private/tmp/dsh_test_dirdsh2>dsh2>dsh2>cmdloopreturned0"

  echo "Captured stdout:"
  echo "Output: $output"
  echo "Stripped: $stripped_output"
  echo "Expected: $expected"

  [ "$stripped_output" = "$expected" ]
  [ "$status" -eq 0 ]

  rmdir /tmp/dsh_test_dir
}

@test "Change directory - no args" {
  current=$(pwd)
  run ./dsh <<EOF
cd
pwd
EOF

  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected="${current}dsh2>dsh2>dsh2>cmdloopreturned0"

  echo "Captured stdout:"
  echo "Output: $output"
  echo "Stripped: $stripped_output"
  echo "Expected: $expected"

  [ "$stripped_output" = "$expected" ]
  [ "$status" -eq 0 ]
}

@test "Return code - successful command" {
  run ./dsh <<EOF
ls
rc
EOF
  [[ "$output" =~ "0" ]]
  [ "$status" -eq 0 ]
}

@test "Return code - failing command" {
  run ./dsh <<EOF
not_a_command
rc
EOF
  [[ "$output" =~ [1-9] ]]
  [ "$status" -eq 0 ]
}

@test "Handles quoted spaces" {
  run ./dsh <<EOF
echo " hello     world "
EOF
  [[ "$output" == *" hello     world "* ]]
  [ "$status" -eq 0 ]
}

@test "Empty input produces no extra output" {
  run ./dsh <<EOF

EOF
  [[ "$output" =~ "dsh2>" ]]
  [ "$status" -eq 0 ]
}

@test "Multiple spaces between arguments" {
  run ./dsh <<EOF
echo    multiple    spaces
EOF
  [[ "$output" =~ "multiple spaces" ]]
  [ "$status" -eq 0 ]
}

@test "Echo with empty quoted string" {
  run ./dsh <<EOF
echo ""
EOF
  [[ "$output" =~ ^$ ]]
  [ "$status" -eq 0 ]
}

@test "Change directory - non-existent directory" {
  run ./dsh <<EOF
cd /nonexistent_directory
EOF
  [[ "$output" =~ "chdir failed" ]] || [[ "$output" =~ "No such file" ]]
  [ "$status" -eq 0 ]
}

@test "Change directory - extra arguments" {
  mkdir -p /tmp/dsh_test_dir2
  run ./dsh <<EOF
cd /tmp/dsh_test_dir2 /var
pwd
EOF

  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected="/private/tmp/dsh_test_dir2dsh2>dsh2>dsh2>cmdloopreturned0"

  [ "$stripped_output" = "$expected" ]
  [ "$status" -eq 0 ]

  rmdir /tmp/dsh_test_dir2
}

@test "Rapid successive commands" {
  run ./dsh <<EOF
echo one
echo two
echo three
EOF
  [[ "$output" =~ "one" ]]
  [[ "$output" =~ "two" ]]
  [[ "$output" =~ "three" ]]
  [ "$status" -eq 0 ]
}

@test "Random input does not crash shell" {
  run ./dsh <<EOF
@#$%^&*()_+
EOF
  [ "$status" -eq 0 ]
}

@test "Maximum argument limit" {
  args=""
  for i in $(seq 1 $CMD_ARGV_MAX); do
    args="$args arg$i"
  done

  run ./dsh <<EOF
echo $args
EOF

  [[ "$output" =~ "arg1" ]]
  [[ "$output" =~ "arg$CMD_ARGV_MAX" ]]
  [ "$status" -eq 0 ]
}
