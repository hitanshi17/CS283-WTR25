1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is a good choice because it reads exactly one line of text at a time (stopping at a newline or the end of the file) and also makes sure it never writes more characters than the buffer can hold, helping you avoid buffer overflows. This aligns perfectly with how a shell handles commands—one line at a time.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  We use malloc() so we can choose the buffer size at runtime instead of being stuck with a fixed size. This lets us handle longer inputs if needed without risking overflow or wasting memory.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Trimming spaces makes sure we don’t accidentally include them as part of the command or arguments. Without trimming, the shell might try to run a command called " ls" (with spaces) instead of just "ls", or include empty arguments where spaces were. This leads to parsing errors and commands not running correctly.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  Common Examples of redirection:
Output Redirection (command > file):
This sends the command’s standard output to a file instead of the terminal.
Input Redirection (command < file):
This makes the command read standard input from a file instead of the keyboard.
Append Redirection (command >> file):
This sends standard output to a file, but instead of overwriting the file, it appends to the end.

Challenges:
When we see symbols like >, >>, or <, we have to split the command correctly to figure out which files are being redirected. We then open those files in the right mode (write, append, or read) and check for errors (in case the file can’t be opened). Next, we use functions like dup2() to switch the standard input or output to those files in a separate process, right before running the command. If we have more than one redirection in a single command, we need to make sure we handle them all in the right order without messing up each other’s file descriptors.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection (>, <, >>) is about sending a command’s input or output to (or from) a file instead of the terminal. Piping (|) is about sending the output from one command directly into the input of another command. In other words, redirection deals with files, while piping connects two commands so they can work together—one command’s output becomes the next command’s input.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  We keep errors (STDERR) separate from normal output (STDOUT) so they don’t get mixed together. If they were combined, it’d be hard to tell what’s an error message and what’s regular output. Having a separate stream for errors makes it easier to spot problems, log them, or handle them differently without confusing them with normal results.


- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  if a command fails, our shell should let its errors go through STDERR so it’s clear something went wrong. Normally, we keep STDOUT (the “good” output) and STDERR (the “error” output) separate, so users can tell them apart. But some users might want to merge both streams (for example, with 2>&1) into one place if they prefer. So, our shell should give the option to either keep them separate or combine them when needed.
s