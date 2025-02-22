1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  We use fork() to create a new child process so that when we call execvp() in the child, it can replace only the memory space of the child process with the new program. This way, the original shell (the parent process) is not corrupted and can continue to accept commands once the child process finishes. Without fork(), calling execvp() directly would replace the shell itself, ending our command loop.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If fork() fails, it returns a negative value (usually -1), which means the system couldn't create a new process—possibly due to resource limits. In our implementation, we check if fork() returns a value less than 0. When that happens, we print an error message (using something like fprintf(stderr, "%s\n", CMD_ERR_EXECUTE)) so that the user knows the command couldn’t be executed, and then the shell continues running instead of crashing.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() tries to find the command by checking if the given command is an absolute path. If not, it attempts to search in directories given in the PATH environment variable. The environment variable tells the system where to search for the executable files, hence execvp() trusts it to locate the command you wish to run.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose of calling wait() in the parent process is to allow the parent to halt execution until the child process finishes. This allows the parent to retrieve the child's exit status and prevents zombie processes. If we had not called wait(), the parent would proceed with execution immediately without waiting for the child's outcome and would leave behind defunct (zombie) processes that occupy system resources.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS() retrieves the exit status from the status given by wait(). It tells us what exit value the child process exited with, which is important to know whether the command had succeeded or failed. It is then utilized for error reporting or handling for internal commands like rc reporting the last command's exit value.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  In my implementation of build_cmd_buff(), I go through the input string character by character and use a flag called in_quotes to know when I’m inside a quoted section. When I hit a double quote and I'm not already in one, I set the flag to true and mark the spot right after the quote as the start of a new argument. Then, when I come across another double quote while in_quotes is true, I turn the flag off, end the current argument, and add that whole chunk to the list of arguments. This is important because it keeps everything inside the quotes together, so something like echo "hello world" ends up with "hello world" as one argument instead of splitting it into two separate parts.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  In the previous assignment, my parser mainly split input on spaces and pipes, without properly handling quoted strings. For Part 2, I simplified it to work on one command at a time and added logic to keep text inside quotes together. The biggest challenge was debugging the quote handling so that tokens weren't split incorrectly. Overall, it was a neat refactoring exercise that helped clean up my old code.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals in Linux are a limited form of interprocess communication used to asynchronously notify a process that an event has occurred—such as a hardware exception, a termination request, or a user-generated interrupt. According to the documentation on man7.org and TLDP, each signal has a default action (like terminating the process), which can be overridden by a custom signal handler. Unlike other IPC mechanisms like pipes or sockets, which allow for the reliable transfer of complex data, signals carry minimal information and serve solely to interrupt a process and prompt immediate action. This design makes them ideal for handling urgent, asynchronous events rather than for regular data communication.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**: 
SIGKILL: A forceful termination signal that cannot be caught, blocked, or ignored.
Typical Use Case: Used to immediately stop a process that isn’t responding or shutting down gracefully.

SIGTERM: A termination signal that can be caught and handled by the process.
TypicalUse Case: Commonly used for a graceful shutdown; it gives a process a chance to clean up resources before exiting.

SIGINT: The interrupt signal, typically generated by pressing Ctrl+C in the terminal.
Typical Use Case: Used to interrupt and terminate a foreground process interactively, allowing it to perform any necessary cleanup.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: When a process receives SIGSTOP, the system immediately halts its execution, and it doesn't proceed until it receives a SIGCONT signal to resume. Although a process can ignore or catch SIGINT, SIGSTOP is different—it cannot be caught, blocked, or ignored. This is intentionally done so that no matter what the process is doing, it will always be able to be stopped when needed, and this is beneficial to system stability and also debugging.







