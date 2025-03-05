1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

The shell uses waitpid() in a loop to wait for every forked child process to finish before moving on. This means the parent shell won't prompt for the next command until it knows all the commands in the pipeline have completed. If waitpid() weren’t called on all child processes, some might finish later or become zombie processes, potentially wasting system resources and causing unpredictable behavior.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

After using dup2(), you need to close the original pipe file descriptors because dup2() duplicates them without closing the originals. If you don't close these unused pipe ends, they can keep the pipe open, preventing the receiving process from getting an end-of-file signal and possibly causing your processes to hang, while also wasting system resources.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

cd is implemented as a built-in because it needs to change the shell’s current working directory, which is a property of the shell process itself. If cd were an external command, it would run in a child process and any directory change would only affect that child, leaving the parent shell’s working directory unchanged. This would require a complex mechanism to communicate the change back to the shell, so implementing cd as a built-in avoids that issue.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To support an arbitrary number of piped commands, you could use dynamic memory allocation (e.g., malloc and realloc) to create an array that grows as needed instead of a fixed-size array. The trade-off is that while this makes your shell more flexible, it adds complexity to your code for managing memory and ensuring all allocations and deallocations are handled properly, as well as potentially impacting performance if the array needs frequent resizing.