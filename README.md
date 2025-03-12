# Extended Shell (eshell)

## Overview
This project is an extended Unix shell implementation, **eshell**, developed as part of the **CENG 334 - Introduction to Operating Systems** course at **Middle East Technical University**. The shell supports various execution modes, including:
- **Sequential Execution** (`;` separator)
- **Parallel Execution** (`,` separator)
- **Pipeline Execution** (`|` separator)
- **Subshell Execution** (`(...)` syntax)

It mimics standard Unix shell behavior while extending its functionality to include parallel execution and advanced subshell operations.

## Features
- **Process Execution**: Supports standard command execution with arguments.
- **Pipeline Handling**: Enables chaining multiple commands using pipes (`|`).
- **Sequential Execution**: Allows sequential execution of commands (`cmd1 ; cmd2`).
- **Parallel Execution**: Runs commands concurrently using the `,` separator (`cmd1 , cmd2`).
- **Subshell Execution**: Groups commands within subshells for isolated execution.
- **Custom Prompt**: Displays a simple `/>` prompt for user input.
- **Graceful Termination**: Supports exiting with the `quit` command.

## Installation & Usage
### Prerequisites
- A Unix-based operating system (Linux/macOS recommended)
- GCC or Clang compiler

### Build Instructions
```sh
make
```
This will compile the `eshell` executable.

### Running eshell
```sh
./eshell
```
Once inside `eshell`, you can execute commands just like in a typical shell environment.

### Example Commands
```sh
/> echo "Hello, World!"
/> ls -l | grep ".c" | wc -l
/> echo "Start" ; sleep 5 ; echo "End"
/> echo "Parallel 1" , echo "Parallel 2" , sleep 3 , echo "Done"
/> (ls -l | grep ".txt" ; echo "Search Done")
```

## Implementation Details
- **Process Management:** Uses `fork()` and `execvp()` to create and execute child processes.
- **Inter-Process Communication:** Implements `pipe()` to facilitate communication between commands in a pipeline.
- **Synchronization:** Uses `wait()` to ensure proper execution flow in sequential and subshell operations.
- **Parallel Execution Handling:** Runs commands concurrently and reaps processes appropriately to prevent zombie processes.
- **Subshell Execution:** Forks a child shell process to execute commands in isolation.

## File Structure
```
├── src/
│   ├── main.c          # Entry point of eshell
│   ├── parser.c        # Parses user input into command structures
│   ├── executor.c      # Handles command execution
│   ├── pipeline.c      # Implements pipeline functionality
│   ├── subshell.c      # Manages subshell execution
│   └── utils.c         # Helper functions
├── include/
│   ├── parser.h        # Header for parsing logic
│   ├── executor.h      # Header for execution logic
│   ├── pipeline.h      # Header for pipeline handling
│   ├── subshell.h      # Header for subshell handling
│   └── utils.h         # Header for utility functions
├── Makefile            # Build instructions
├── README.md           # Project documentation
```

## Future Improvements
- Add support for background process execution (`&` operator).
- Implement I/O redirection (`>`, `<`, `>>`).
- Enhance error handling and logging.

## License
This project is licensed under the **MIT License**.

## Author
Developed as part of **CENG 334 - Introduction to Operating Systems** coursework at **Middle East Technical University (METU)**.

