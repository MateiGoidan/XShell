#include "xvar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_PATH 1030

void parse_command(char *input, char **args) {
  /* Imparte input-ul in command + arguments (tokens) */

  if (strchr(input, '=') != NULL && strchr(input, '=') == strrchr(input, '=')) {
    char *name = strtok(input, "=");
    char *value = strtok(NULL, "=");

    args[0] = "set";
    args[1] = name;
    args[2] = value;
    args[3] = NULL;

    return;
  }

  int i = 0;
  args[i] = strtok(input, " "); // Primul token

  while (args[i] != NULL) {
    args[++i] = strtok(NULL, " "); // Urmator-ul token
  }
}

int exec_command(char **args) {
  if (!strcmp(args[0], "set") && args[1] && args[2]) {
    set_variable(args[1], args[2]);
    return 1;
  }

  if (!strcmp(args[0], "unset") && args[1]) {
    unset_variable(args[1]);
    return 1;
  }

  return 0;
}

int main() {
  char input[MAX_INPUT];
  char *args[MAX_ARGS];

  while (1) {
    printf("XShell> ");

    // Ignora cazul in care input-ul este gol
    if (fgets(input, MAX_INPUT, stdin) == NULL) {
      continue;
    }

    // Inlatura "\n" de la sfarsitul liniei
    input[strlen(input) - 1] = '\0';

    // Iesire shell
    if (strcmp(input, "exit") == 0) {
      break;
    }

    parse_command(input, args);

    // printf("Command: %s\n", args[0]);
    // int i = 1;
    // if (args[i] != NULL) {
    //   printf("Arguments: ");
    //   do {
    //     printf("%s ", args[i++]);
    //   } while (args[i] != NULL);
    //   printf("\n");
    // }

    if (exec_command(args)) {
      continue;
    }

    pid_t pid = fork();
    if (pid == 0) {
      execvp(args[0], args);
      perror("Error: exec failed!\n");
      exit(1);
    } else if (pid > 0) {
      wait(NULL);
    } else {
      perror("Error: Fork didn't work!\n'");
    }
  }

  return EXIT_SUCCESS;
}
