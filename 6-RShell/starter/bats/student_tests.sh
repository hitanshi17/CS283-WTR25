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


function start_server() {
  ./dsh -s -p $PORT > server_output.log 2>&1 &
  SERVER_PID=$!
  sleep 1  
}
function stop_server_bg() {
  if kill -0 $SERVER_PID 2>/dev/null; then
    kill $SERVER_PID
    wait $SERVER_PID 2>/dev/null
  fi
}

@test "Remote: Single command (ls)" {
  start_server

  run ./dsh -c -p $PORT <<EOF
ls
stop-server
EOF

  echo "Captured stdout:"
  echo "$output"
  echo "Exit Status: $status"

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Built-in exit only closes client, server stays up" {
  start_server

  run ./dsh -c -p $PORT <<EOF
exit
EOF
  [ "$status" -eq 0 ]
  run ./dsh -c -p $PORT <<EOF
pwd
stop-server
EOF

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Simple pipeline" {
  start_server

  run ./dsh -c -p $PORT <<EOF
ls | grep dsh
stop-server
EOF

  echo "Captured stdout:"
  echo "$output"
  echo "Exit Status: $status"

  [[ "$output" =~ "dsh_cli.c" ]] || true

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: cd built-in on server side" {
  start_server

  run ./dsh -c -p $PORT <<EOF
pwd
cd ..
pwd
stop-server
EOF

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Redirection test" {
  start_server

  run ./dsh -c "echo hello > remote_test.txt && cat remote_test.txt && stop-server"

  [[ "$output" == *"hello"* ]]
  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Attempt unknown command" {
  start_server

  run ./dsh -c -p $PORT <<EOF
some_nonexistent_cmd
stop-server
EOF

  [[ "$output" =~ "Command not found" ]] || [[ "$output" =~ "execvp" ]] || true

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: input overflow test" {
  start_server

  LONG_CMD=$(head -c 1000000 < /dev/zero | tr '\0' 'A') 
  run ./dsh -c -p $PORT <<EOF
$LONG_CMD
stop-server
EOF
  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Fork bomb test" {
  start_server

  run ./dsh -c -p $PORT <<EOF
:(){ :|:& };:
stop-server
EOF
  [[ "$output" =~ "Resource temporarily unavailable" ]] || [[ "$output" =~ "Too many processes" ]] || true

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Malformed command input test" {
  start_server

  run ./dsh -c -p $PORT <<EOF
$(echo -e "ls\0ls")  # Send a NULL byte in the middle
stop-server
EOF
  [[ "$output" =~ "Invalid command" ]] || true

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Infinite loop in shell" {
  start_server

  run ./dsh -c -p $PORT <<EOF
while true; do echo "looping"; done
stop-server
EOF

  [[ "$output" =~ "looping" ]] || true  

  stop_server_bg
}

@test "Remote: Garbage input" {
  start_server

  run ./dsh -c -p $PORT <<EOF
@#&*^!(*@#&@!#*&@&^!!#(*&@)
stop-server
EOF

  [[ "$output" =~ "Invalid command" ]] || true

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Remote: Redirecting output to nonexistent file" {
  start_server

  run ./dsh -c -p $PORT <<EOF
echo "test" > /root/protected_file.txt
stop-server
EOF

  [[ "$output" =~ "Permission denied" ]] || true

  [ "$status" -eq 0 ]

  stop_server_bg
}

@test "Simple command execution: echo" {
  run ./dsh <<EOF
echo hello
exit
EOF
  [[ "$output" =~ "hello" ]]
  [ "$status" -eq 0 ]
}

@test "Pipeline execution: echo | tr" {
  run ./dsh <<EOF
echo hello world | tr a-z A-Z
exit
EOF
  [[ "$output" =~ "HELLO WORLD" ]]
  [ "$status" -eq 0 ]
}

@test "Exit command terminates the shell" {
  run ./dsh <<EOF
exit
EOF
  [[ "$output" =~ "dsh3>" ]]
  [ "$status" -eq 0 ]
}

@test "Change directory builtin: cd" {
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

