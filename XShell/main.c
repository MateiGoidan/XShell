#include "xvar.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
// #define MAX_PATH 1030

void parse_command(char *input, char **args) {
  /* Imparte input-ul in argumente si expandeaza variabilele gasite */

  // Initializarea / Reinitializarea variabilei
  if (strchr(input, '=') != NULL && strchr(input, '=') == strrchr(input, '=')) {
    char *name = strtok(input, "=");
    char *value = strtok(NULL, "=");

    args[0] = "set";
    args[1] = name;
    args[2] = value;
    args[3] = NULL;

    return;
  }

  // Comanda bash
  int i = 0;
  args[i] = strtok(input, " ");

  while (args[i] != NULL) {
    if (strchr(args[i], '$')) {
      // Verificam daca apare o variabila
      char arg_copy[MAX_INPUT] = "";
      char *pos = args[i];

      while (*pos) {
        if (*pos == '$') {
          // Gasim o variabila, care incepe cu '$'
          char name[100];
          ++pos;

          int j = 0;
          while (*pos && (isalnum(*pos) || *pos == '_')) {
            // Citim numele complet al variabilei
            name[j++] = *pos++;
          }
          name[j] = '\0';

          // Inlocuim cu valoarea variabilei daca exista
          char *value = strdup(get_value(name));
          if (value) {
            strcat(arg_copy, value);
          }
        } else {
          // Nu gasim o variabila
          strncat(arg_copy, pos, 1);
          ++pos;
        }
      }
      strcpy(args[i], arg_copy);
    }

    args[++i] = strtok(NULL, " ");
  }
}

void exec_command(char **args) {
  /* Proceseaza input-ul ca pe o comanda normala */

  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    printf("Error: Unknown command %s\n", args[0]);
    exit(1);
  } else if (pid > 0) {
    wait(NULL);
  } else {
    perror("Error: Fork didn't work!\n'");
  }
}

int var_command(char **args) {
  /* Executa comenzi specifice varibilelor */

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

int run_script(char **args) {
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

    // Ignora cazul in care input-ul este doar "\n"
    if (!strcmp(input, "\n")) {
      continue;
    }

    // Inlatura "\n" de la sfarsitul liniei
    input[strlen(input) - 1] = '\0';

    // Iesire shell
    if (strcmp(input, "exit") == 0) {
      break;
    }

    parse_command(input, args);

    // Comanda specifica variabilelor
    if (var_command(args)) {
      continue;
    }

    // Script Bash
    if (run_script(args)) {
      continue;
    }

    // Comanda Bash
    exec_command(args);
  }

  return EXIT_SUCCESS;
}
