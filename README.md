# XShell

XShell is a simple shell implemented in C, supporting variable assignment, script execution, and environment expansion, all without relying on the system shell.

----------

## Features

- Internal command: `exit`
- Execution of external commands with arguments
- Shell variable definition and substitution:
  - Assign: `name=value`
  - Use: `$name`
  - Remove: `unset name`
- Automatic export of shell variables to child processes (`setenv`)
- Execution of `.sh` scripts (custom format, not bash)
  - Supports arguments via `$1`, `$2`, ...
- Variable and argument substitution inside scripts

## Buid Instructions

Make sure you have `gcc` installed, then:

```bash
make        # Compiles the shell
make run    # Runs the shell
make clean  # Cleans the executable
```

- The compiled shell is placed in **`XShell/bin/my_shell`**

## Project Structure

```python
project-root/
├── Makefile
└── XShell/
    ├── main.c        # Core shell logic
    ├── xvar.c        # Variable management (set, get, unset)
    ├── xvar.h        # Variable definitions & function declarations
    └── bin/          # Output directory for the compiled shell
``` 

## Usage Example

```sh
XShell> myname=Matei
XShell> echo $myname
Matei
```
