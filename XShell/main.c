#include "xvar.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ANSI_COLOR_GREEN "\033[3;92m"
#define ANSI_COLOR_SUCCESS "\x1b[32m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\033[0m"

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_SCRIPT_DEPTH 32

int script_depth;

void var_expand(char **input_ptr, char *arg_copy) {
  // Expand a variable that starts with '$'
  char *input = *input_ptr;
  char name[100] = "";
  ++input;

  if (*input == '$') {
    strcat(name, "$");
  } else {
    int j = 0;
    while (*input && (isalnum(*input) || *input == '_')) {
      // Read the full name of the variable
      name[j++] = *input++;
    }
    name[j] = '\0';
  }

  // Replace with the value of the variable if it exists
  char *value = get_value(name);
  if (value) {
    strcat(arg_copy, value);
  }

  // "$$" case
  if (!strcmp(name, "$")) {
    ++input;
  }

  *input_ptr = input;
}

void parse_command(char *input, char **args) {
  /* Split the input into arguments */
  int index = 0;
  char arg_copy[MAX_INPUT] = "";

  while (*input) {
    // End of input
    if (*input == '\0') {
      ++index;
      break;
    }

    // Ignore spaces between arguments
    while (*input == ' ') {
      ++input;
    }

    arg_copy[0] = '\0';

    // Traverse the argument
    while (*input && *input != ' ') {
      if (*input == '"') {
        ++input;

        while (*input != '"') {
          if (*input == '$') {
            var_expand(&input, arg_copy);
          } else if (*input == '\0') {
            // Error: Unclosed double quotes
            printf(ANSI_COLOR_RED "Error: Unclosed double quotes!\n" ANSI_COLOR_RESET);
            args[0][0] = '\0';
            return;
          } else {
            strncat(arg_copy, input, 1);
            ++input;
          }
        }

        if (*input == '"') {
          ++input;
        }
      } else if (*input == '$') {
        var_expand(&input, arg_copy);
      } else {
        // Not a variable
        strncat(arg_copy, input, 1);
        ++input;
      }
    }

    args[index] = strdup(arg_copy);
    if (args[index] == NULL) {
      perror("Error: strdup failed (args[i])!\n");
      exit(EXIT_FAILURE);
    }

    ++index;
  }

  args[index] = NULL;
}

int exec_command(char **args) {
  /* Process the input as a normal command */

  // Skip over variable declarations
  int i = 0;
  while (args[i] && strchr(args[i], '=') != NULL &&
         strchr(args[i], '=') == strrchr(args[i], '=')) {
    ++i;
  }

  if (args[i] == NULL) {
    return -1;
  }

  pid_t pid = fork();
  if (pid == 0 && args[i]) {
    execvp(args[i], &args[i]);
    printf(ANSI_COLOR_RED "Error: Unknown command %s\n" ANSI_COLOR_RESET, args[i]);
    exit(1);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0); // Save child output

    if (WIFEXITED(status)) {
      return 0;
    } else {
      return -1;
    }
  } else {
    perror("Error: Fork didn't work!\n'");
    return -1;
  }
}

void var_command(char **args, int visible) {
  /* Execute variable-specific commands */

  // Unset variables
  if (!strcmp(args[0], "unset") && args[1] && strcmp(args[1], "")) {
    for (int i = 1; args[i] != NULL; i++) {
      if (!unset_variable(args[i]) && visible) {
        printf(ANSI_COLOR_YELLOW "Warning: Variable '%s' not found.\n" ANSI_COLOR_RESET, args[i]);
      }
    }
    return;
  }

  // Set variables
  for (int i = 0; args[i] != NULL; i++) {
    if (strchr(args[i], '=') != NULL &&
        strchr(args[i], '=') == strrchr(args[i], '=')) {
      char name[MAX_INPUT] = "";
      char value[MAX_INPUT] = "";

      int j = 0;
      while (args[i][j] != '=') {
        name[j] = args[i][j];
        ++j;
      }

      if (args[i][j] == '=') {
        ++j;
      }

      int k = 0;
      while (args[i][j] != '\0') {
        value[k++] = args[i][j++];
      }

      if (set_variable(name, value) && visible) {
        printf(ANSI_COLOR_SUCCESS "Notice: Variable '%s' allocated.\n" ANSI_COLOR_RESET, name);
      }
    }
  }
}

void free_args(char **args) {
  /* Free the memory allocated for arguments */
  for (int i = 0; args[i] != NULL; i++) {
    free(args[i]);
    args[i] = NULL;
  }
}

int run_script(char **args) {
  /* Execute bash script */

  if (strlen(args[0]) > 3 &&
      strcmp(args[0] + strlen(args[0]) - 3, ".sh") == 0) {
    if (access(args[0], F_OK) == -1) {
      // Check if the script file exists
      fprintf(stderr, ANSI_COLOR_RED "Error: File '%s' not found!\n" ANSI_COLOR_RESET, args[0]);
      return 1;
    }

    // Check if maximum recursion depth is reached
    if (script_depth >= MAX_SCRIPT_DEPTH) {
      fprintf(stderr, ANSI_COLOR_RED "Error: Maximum script recursion depth reached!\n" ANSI_COLOR_RESET);
      return 1;
    }

    // Initialize command-line arguments
    for (int i = 0; args[i] != NULL; i++) {
      char digit[12];
      sprintf(digit, "%d", i);
      set_variable(digit, args[i]);
    }

    FILE *script = fopen(args[0], "r");
    if (!script) {
      perror(ANSI_COLOR_RED "Error: Failed to open script!\n" ANSI_COLOR_RESET);
      return 1;
    }

    // Execute all lines
    char line[MAX_INPUT];
    while (fgets(line, sizeof(line), script)) {
      char *arguments[MAX_ARGS];

      // Skip comments
      if (line[0] == '#') {
        continue;
      }

      // Skip empty lines
      if (!strcmp(line, "\n")) {
        continue;
      }

      line[strlen(line) - 1] = '\0';

      parse_command(line, arguments);

      if (!strcmp(arguments[0], "\0")) {
        continue;
      }

      if (strcmp(arguments[0], "exit") == 0) {
        free_args(arguments);
        for (int i = 1; args[i] != NULL; i++) {
          char digit[12];
          sprintf(digit, "%d", i);
          unset_variable(digit);
        }
        return 2;
      }

      script_depth++;

      // Bash
      if (strcmp(arguments[0], "unset")) {
        // Script
        int output = run_script(arguments);
        if (output == 1) {
          free_args(arguments);
          continue;
        } else if (output == 2) {
          free_args(arguments);
          for (int i = 1; args[i] != NULL; i++) {
            char digit[12];
            sprintf(digit, "%d", i);
            unset_variable(digit);
          }
          return 2;
        }

        // Command
        int code = exec_command(arguments);
        if (code == 0) {
          free_args(arguments);
          continue;
        }
      }

      var_command(arguments, 0);

      free_args(arguments);
    }

    // Remove variables passed as arguments
    for (int i = 1; args[i] != NULL; i++) {
      char digit[12];
      sprintf(digit, "%d", i);
      unset_variable(digit);
    }

    return 1;
  }

  return 0;
}

int main() {
  char input[MAX_INPUT];
  char *args[MAX_ARGS];

  // Set the PID variable
  char pid_str[20];
  pid_t pid = getpid();
  snprintf(pid_str, sizeof(pid_str), "%d", pid);
  set_variable("$", pid_str);

  while (1) {
    printf(ANSI_COLOR_GREEN "XShell> " ANSI_COLOR_RESET);

    // Ignore empty input
    if (fgets(input, MAX_INPUT, stdin) == NULL) {
      continue;
    }

    if (!strcmp(input, "\n")) {
      continue;
    }

    // Remove the trailing "\n" from the end of the line
    input[strlen(input) - 1] = '\0';

    parse_command(input, args);

    // Ignore input that is only empty spaces
    if (!strcmp(args[0], "\0")) {
      continue;
    }

    // Exit shell
    if (strcmp(args[0], "exit") == 0) {
      free_args(args);
      free_variables();
      printf("\n");
      break;
    }

    // Bash
    if (strcmp(args[0], "unset")) {
      // Script
      int output = run_script(args);
      if (output == 1) {
        free_args(args);
        script_depth = 0;
        continue;
      } else if (output == 2) {
        free_args(args);
        free_variables();
        script_depth = 0;
        printf("\n");
        break;
      }

      // Command
      int code = exec_command(args);
      if (code == 0) {
        free_args(args);
        continue;
      }
    }

    // Variables
    var_command(args, 1);

    free_args(args);
  }

  return EXIT_SUCCESS;
}
