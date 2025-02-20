# BigShell Project

## Abstract
BigShell is a POSIX-like shell program written in the C programming language [1]. Skelton code is provided to begin this assignment, which includes a makefile, bigshell.c file, and several other ancillary files that each include a .c file and a header file (which defines the interface) [2]. Some of the provided files do not get modified, as they support the completion of the assignment, while others require significant additions. A main focus of this assignment is on the execution of parsed commands. 
The overall functionality of the BigShell program includes seven key behaviors: parsing command-line input into commands, executing external commands, implementing built-in commands, performing I/O redirection, assigning shell variables (in addition to exporting and/or evaluating), implementing signal handling, and managing processes and pipelines with job control [1]. The BigShell program must be able to handle commands written in the BigShell Command Language, which provides its own syntax and rules for user commands and fits closely with a POSIX-compliant shell language, just with some missing features [1].
Through implementing BigShell, many important operating system concepts are explored. This includes a stronger grasp of the C programming language and related concepts, like structures, pointers, libraries, and header files. Importantly, this also includes a deep understanding of processes, including creating new processes through the fork() and exec() calls, permissions, id numbers (e.g. pgid), states, signals, and job and session control [3]. Another important concept for which an understanding is required is standard streams (i.e. stdin, stdout, stderr), which include the important and relevant concepts of shell redirection and shell pipelines [4]. Finally, a wide range of system calls must be understood and utilized. Overall, the BigShell project provides a comprehensive, hands-on experience with core operating system concepts, reinforcing both knowledge of and practical use of concepts necessary for working with processes, I/O, and shell environments [1].

 
## I. INTRODUCTION
In an operating system, the kernel is the central software that manages and allocates computer resources [5, pp.23]. The kernel has access to kernel mode on the CPU and kernel space in memory [5], [6]. On the other hand, user processes operate in user mode and user space [5]. When a user process wants to create a new process or write data to a file, it must request these actions from the kernel [5, pp.24]. So, how does a process go about asking the kernel anything? This is where a very important user-space process comes into play- the shell. A shell is a command-line interface that allows users to interact with an operating system, acting as a bridge between user space and kernel space [7]. The shell “allows users to navigate the file system, run programs, manage processes, and perform various system tasks” [7].

While a graphical interface (GUI) may offer a simpler way for users to interact with the system, the shell offers greater control and flexibility, particularly for advanced users like system administrators or programmers [8]. Common shells include the Bourne Shell (sh) and the Bourne Again Shell (bash) [5, pp.25], both of which are based on the Portable Operating System Interface (POSIX) Shell Command Language [7]. 

BigShell is a user-program inspired by the shell command language specification in POSIX.1-2008 [1]. It is designed to provide an interface for interacting with the operating system’s kernel, similar to sh or bash. While some features are simplified or removed due to time and complexity constraints, BigShell still includes many core functionalities found in standard shells [5]:
-Parsing of command-line input into commands to be executed [1]
-Executing of external commands as separate processes [1]
-Implementing of a variety of shell built-in commands within the shell itself [1]
-Performing I/O redirections [1]
-Manipulating environment variables [1]
-Handling signals for the shell and executed commands [1]
-Managing processes and pipelines using job control [1]
-The process of implementing BigShell, with much of a POSIX’s Shell’s functionality, lends to a deeper understanding of Unix-like file systems, process management, signal handling, I/O redirection, and more, all of which are essential for understanding the underlying operating system [1]. 

## II. PROGRAM STRUCTURE
BigShell is designed as a POSIX-like shell, with its structure divided into several modular components that interact to process and execute user commands. The file bigshell.c contains the main function for the program, which is the entry point of BigShell [2]. When main() is called, in this case by a parent shell, it first does some initializations. First, an instance of the command_list structure is initialized to empty, as no commands have yet been received. Then, it checks to see if the program is running in interactive mode (connected to the terminal) or non-interactive mode (reading input from a file or pipe). And finally, signal handling is initialized. 

After initialization, BigShell enters an infinite for loop, called the REPL (Read, Evaluate, Print Loop), which handles the core functionality of the command line interface [9], [2]. The loop first ensures that any background jobs from previous iterations are managed appropriately. 

From here, the shell gets ready to read input and parse it into a list of commands [9]. But first, the SIGINT signal needs to be turned on to allow for a user to be able to interrupt (e.g. via Ctrl+C) the read operation and prompt for a new command. After this, BigShell displays a simple prompt and retrieves commands into the command_list structure [2]. Once the input is successfully parsed, the SIGINT signal is turned back off to ignore any interrupts for the remainder of the loop. Next, any errors that may have occurred in retrieving commands, including system library errors and parser syntax errors, are handled [9]. If the command-line input is parsed successfully into commands[1], the program moves on to execution. 

BigShell executes the commands by calling the run_command_list function, passing the command_list structure. The first step of running a command is creating an instance of the pipeline_data structure, which stores key information for managing pipelines, process groups, and job control. This instance persists across commands in a pipeline, allowing data (e.g. file descriptors) to be passed between commands [9]. 

Now, the function iterates over each command in the command_list and executes each one in the proper sequence [9]. The process for each command is as follows:
- **Expansion:** Each command undergoes expansion, where tildes are expanded, quotes are removed, and symbols are substituted with their corresponding values. This applies to the command name, any arguments, assignments, and filenames for I/O redirection; all of which have been stored in the command_list structure [2], [9].
- **Flags:** Flags are set to determine the type of command being run based on the control operator at the end of the command (stored in the ctrl_op). These flags indicate if the command is foreground (‘;’), background (‘&’), or part of a pipeline (‘|’) [9].
- **Pipeline Handling:** Since the command itself is oblivious to whether or not it is part of a pipe, the function prepares to read data from the pipeline of the previous command [9]. This data, which is the file descriptor for the upstream end of the pipe, will still be saved in the pipeline_data struct from the previous command, if it exists. If the previous command was not a pipe, then the command will get its input from stdin. Next, the shell checks if the current command has a pipeline operator, and if so, it creates a new pipe. The write end of this new pipe is connected to the stdout of this current command, while the read end of the pipe that was just created is saved for the next command in the list to use [5]. 
- **Built-in Commands:** If the current command is a built-in command within BigShell (i.e. cd, exit, fg, bg, jobs, unset, or export), then a function pointer to that built-in function is saved. If the current command is not a built-in, then it is an external command. A flag will be set to save this information (i.e. built-in or not).
- **Forking:** If the current command is an external command or background command, then the shell will fork and create a child process. From this point on, there are two processes: the parent (the shell) and the child. If the command is the first in a pipeline, the parent process sets the child as the leader of a new process group. All subsequent commands in the pipeline will share the same process group ID (pgid), allowing them to be treated as a single unit for job control purposes [5], [3]. The shell also records the job in the job table, which enables it to manage and send signals to all processes within the pipeline [9]. If the command is not part of a pipeline, then it will receive a unique pgid.
- **Command Execution:** 
 - **Built-In Command:** If the current command is a built-in, then flags will be checked to handle potential upstream or downstream pipes, then redirection is handled, and variable assignment is handled. Then the built-in is called and run. In the strange case that the built-in is running in the background, and is therefore a child process, it will exit. Otherwise, the built-in command is running in the current shell and needs to clean up before falling through [2].
 - **External Command:** If not a built-in, the current command is an external command. Pipelines from the command are handled, I/O redirections are performed, and variable assignments are made. Right before executing the command, the signal dispositions are restored for the child process, so that they aren’t ignored (which is the default behavior for the parent) [2]. Finally, the command is executed by searching for the correct executable file in the PATH environment variable [9]. From here, the child process will not return unless an exec error occurred; in which case, the error is handled and the child exits.
- **Parent Process Handling:** From here, only the parent process, the shell, will execute the remaining code. The shell will close pipeline file descriptors that were opened and have been already hooked up [2], [9]. Then, it handles foreground and background commands. To prevent an orphaned process, the parent waits on any children that are foreground processes to complete. If the process is a background or pipeline process, it assigns the child process ID to the background process ID parameter. If the command is a background process, it also prints the job ID and group ID to stderr. Additionally, if the command was not part of a pipeline, the parent resets the pipeline data [9].
- **Repeat** with next command, if any.

Finally, after completing this loop for each command in the command_list, the program returns to the main loop, cleaning up and preparing for the next set of commands. If at any point within the above loop an exit condition is encountered (e.g. exit command or end of input), the shell exits the loop and terminates [2].

## III. IMPLEMENTATION DETAILS
I began the development process by forking the BigShell assignment repository on GitHub, which provided starter code that included source files and a makefile. I used **GitHub Codespaces** exclusively as my IDE, which has a **VS Code** interface with pre-installed tools like the **gcc compiler** and **git**.

My portion of the BigShell implementation was written entirely in **C**. I used the provided starter code as a foundation for implementing the project. Files like `exit.c`, `expand.c`, `jobs.c`, and `parser.c` were already fully implemented to handle:
- Cleanup
- Managing the list of jobs
- Storing globally accessible values that must be used by the shell in different locations
- Parsing input into a command list [2].

The provided **makefile** was key in managing the build process. At a high level, the makefile included two primary targets: **release** and **debug**. The makefile used a template for building rules dynamically to create the two different executables. Both builds shared the same source files [2], along with initial compiler flags (`CFLAGS`), like using the **C99** standard and enabling all common warnings [11]. The two builds were distinct, allowing easy switching between versions of the shell.

### Key Differences Between Builds
- **Release Build**:
  - Added `-O3` CFLAG (optimizations) [11].
  - Added `-DNDEBUG` CPPFLAG to disable assertions and ignore special `gprint` functions for printing debugging information [12][2].
  - Output was smaller, excluding debugging information.
  
- **Debug Build**:
  - Added `-O0` CFLAG (no optimizations) and `-g` CFLAG to include debug symbols for debugging [11].
  - Output was longer with extra print statements and debugging information.

Once these builds were created, only the changed files needed to be recompiled and re-linked by **make** [2]. This saved significant time compared to manually typing out compiler and linker commands after every change [8].

For version control, I relied on **git** to track changes. After making modifications, I would use **make** to rebuild the executable and then commit and push the changes to the repository. Testing was handled through **GradeScope**, where I regularly submitted my code. GradeScope ran automated tests that checked BigShell against a set of specifications. Over **24 submissions**, I was able to incrementally improve my score.

The most common resources I used for research were:
- The **C Programming Language** [13]
- The **Linux Programming Interface** [5]
- The **CS 374 modules and videos**
- **Linux man pages**

In addition to GradeScope testing, I frequently tested my program manually using the debug build. I ran commands in the terminal and verified the functionality of all features, including:
- Built-ins
- External commands
- Redirection
- Pipelines
- Job control
- Signal handling

With this combination of research, testing, and debugging, I was able to successfully implement the following functions in BigShell:

### Built-in Shell Utilities:
- **builtin_cd**: Changes the working directory and updates the **PWD** variable accordingly. This involved handling cases with zero or one argument and managing potential errors.
- **builtin_exit**: Sets the appropriate exit status by converting a string argument (if provided) to an integer and assigning it to `params.status` before exiting.
- **builtin_unset**: Removes a list of shell variables.

### Signal Handling:
Using a provided signal handler template, I implemented the shell’s signal handling behavior with the following functions:
- **signal_init**: Changes the signal disposition of **SIGTSTP**, **SIGINT**, and **SIGTTOU** signals (while saving their old actions) by using `sigaction()` [5].
- **signal_enable_interrupt**: Sets the signal disposition for a given signal to interrupt.
- **signal_ignore**: Sets the signal disposition for a given signal to be ignored by the shell.
- **signal_restore**: Restores the signal dispositions to their old state saved by **signal_init**.

### Command Execution:
To aid in the execution of commands, I added logic to complete several helper functions, mostly in **runner.c**:
- **expand_command_words**: Expands command words, assignment variables, and redirection filename operators by appropriately calling `expand()` for each command word.
- **do_variable_assignment**: Performs variable assignments before running commands and exports them if the **export** flag is set.
- **get_io_flags**: Returns the appropriate flags for the given I/O redirection operators: `>`, `<`, `<>`, `>>`, `>|`, `>&`, and `<&`—selected based on the **BigShell Command Language** specification for redirection [1], [3].
- **move_fd**: Moves a file descriptor. Specifically, moves `src` to `dst`, and closes `src`.
- **do_io_redirects**: Performs I/O redirection for non-built-in commands. It iterates over the list of redirections and applies each one in sequence. This implementation handles closing and duplicating file descriptors and ensures proper error handling.

In **vars.c**, I also implemented the function `is_valid_varname` to ensure variable names match the **POSIX.1-2008** specification. Additionally, I implemented `vars_is_valid_varname` to validate inputs before calling `is_valid_varname()`.

In **wait.c**, I completed the function `wait_on_fg_pgid`, which included:
- Managing the foreground process group
- Handling process termination or stops
- Updating exit statuses

### run_command_list() Function:
The functions mentioned above (under **command execution**) are all called in the `run_command_list()` function. The majority of my implementation focused inside this function and the functions it called.

I implemented the following functionality into the skeleton code:
- **Pipeline Handling**: Includes the initialization of upstream and downstream pipes, creating new pipes, and closing unused pipes after a child process has forked.
- **Process Management**: Includes determining when to fork and implementing the actual `fork()` call. It also sets up process groups for job control and manages how the parent waits for the child process using the functions in **wait.c**.
- **Redirection**: Handles file descriptor redirection for pipes.
- **Command Execution**: Implements `execvp()` for external command execution [3].

## IV. CHALLENGES AND SOLUTIONS
The most challenging part of development was figuring out how and where to begin. As someone new to the C programming language within the last three months, I found working with multiple C source files incredibly intimidating. Getting used to compiling before debugging was also an adjustment, as I’m used to programming in Python, but the instructor videos on make helped with running and debugging the program. The next, and most important, thing I did to get started, was I drew out a flowchart. The most logical place to begin was the entry point to BigShell, so I started with the main() function in bigshell.c. I mapped out its logic, tracked which files each function came from, and color-coded the function calls to match the file they were in. 

Following the logic of main(), I discovered that much of the implementation centered on the run_command_list function, which relies heavily on the command_list structure in parser.h. At first, this structure was incredibly challenging to understand due to its three levels of nested structures and use of double pointers. I revisited The C Programming Language [13] to review nested structures and pointers. I started by drawing out the innermost levels (assignment and io_redir structures), then worked outward. This approach took time but ultimately paid off, as it allowed me to piece together how commands were stored, accessed, parsed, and executed.

For me, the most challenging topics were pipelines and process management, which were completely new concepts to me. For pipelines, The Linux Programming Interface book had many examples in chapter 44 [5], which helped, along with the Linux man pages. In terms of process management, I had difficulty with the wait_on_fg_pgid function. I finally resolved these issues through countless hours of reading documentation, especially around the waitpid() syscall, creating a very detailed flowchart for the function, and seeking guidance from Ed Discussion posts and office hours. 
From this project, I’ve learned to tackle large, intimidating tasks by breaking them into manageable pieces, using flowcharts to map out logic, and dedicating time to research with reliable resources. Moving forward, I’ll continue applying these strategies, but now with a newfound confidence in my abilities. Completing this project, despite my initial doubts, proved that I am capable of overcoming complex challenges.

## V. CONCLUSION
The development of BigShell provided a hands-on opportunity to explore many important operating system concepts. This project offered immense practice with the C programming language, including the use of structures, pointers, and libraries. It also required a strong understanding of processes, particularly the use of fork() and exec() to create new processes, managing process states, signals, and job control. Additionally, it required immense knowledge of standard streams, which were critical in implementing I/O redirection and pipelines. Finally, cementing an understanding of how shell environments interact with the kernel was the frequent need for and use of a variety of system calls. Together, this implementation of BigShell, was the practical application needed to bring my own knowledge of a shell from using a few simple commands in a shell, to understanding how a shell actually executes those commands.
BigShell currently has many of the same features as other POSIX-like shells, but still, several features were removed or simplified [1]. Further improvement for BigShell could focus on enhancing usability by supporting command history [5] and the use of the arrow keys within the shell (e.g. to reuse a previous command) [8]. Additionally, more bash-like features could be included, like support for filename generation (globbing), aliases, and loops [5].

## REFERENCES
[1]      R. Gambord, "BigShell Specification." Operating Systems I. Accessed: Nov 22, 2024.  [Online]. Available: https://rgambord.github.io/cs374/assignments/bigshell/

[2]      R. Gambord. BigShell Introduction Video. (July 17, 2024). Accessed: Nov 23, 2024. [Online Video]. Available: https://www.youtube.com/watch?v=mCl_v94_n9I

[3]      G. Tonsmann. (2024). Lecture 6: Files for Input and Output [pdf]. Available: https://canvas.oregonstate.edu/courses/1975687/files/107972071?module_item_id=24919331

[4]      R. Gambord, “Standard Streams.” Operating Systems I. Accessed: Dec 1, 2024. [Online]. Available: https://rgambord.github.io/cs374/assignments/base64/streams/

[5]      M. Kerrisk, The Linux Programming Interface. San Francisco, California, USA: No Starch Press, Inc., 2010. 

[6]      D. P. Bovet and M. Cesati, Understanding the Linux Kernel, 3rd ed. Sebastopol, CA, USA: O’Reilly Media, Inc., 2006. [Online]. Available: https://github.com/theja0473/My-Lib-Books-1/blob/master/UnderStanding%20The%20Linux%20Kernel%203rd%20Edition%20V413HAV.pdf. Accessed: November 24, 2024.

[7]      R. Gambord, “Utilities” Operating Systems I. Accessed: Nov 20, 2024. [Online]. Available: https://rgambord.github.io/cs374/modules/01/utilities/

[8]      B. Ward, How Linux Works, 2nd ed. San Francisco, California, USA: No Starch Press, Inc., 2015. [Online]. Available: https://ebookcentral.proquest.com/lib/osu/reader.action?docID=1848073. Accessed: Nov 24, 2024.

[9]      R. Gambord. BigShell. (2024). Accessed: Nov 5, 2024. [Online].

[10]    R. Mecklenburg, Managing Projects with GNU Make, 3rd ed. Sebastopol, CA, USA: O’Reilly Media, Inc., 2005. [Online]. Available: http://uploads.mitechie.com/books/Managing_Projects_with_GNU_Make_Third_Edition.pdf. Accessed: Nov 24, 2024.

[11]    “Options That Control Optimization” GCC, the GNU Compiler Collection. Accessed: Nov 20, 2024. [Online]. Available: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html

[12]    “Options Controlling the Preprocessor” GCC, the GNU Compiler Collection. Accessed: Nov 20, 2024. [Online]. Available: https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html

[13]    B.W. Kernighan and D.M. Ritchie, The C Programming Language, 2nd ed. Murray Hill, New Jersey, USA: Pearson Education, Inc., 1988.
