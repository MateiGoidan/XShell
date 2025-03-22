#include "xvar.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ANSI_COLOR_GREEN "\033[3;92m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\033[0m"

#define MAX_INPUT 1024
#define MAX_ARGS 64

void parse_command(char *input, char **args, int *duplicated) {
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
          char *value = get_value(name);
          if (value) {
            strcat(arg_copy, value);
          } else {
            strcat(arg_copy, args[i]);
          }
        } else {
          // Nu gasim o variabila
          strncat(arg_copy, pos, 1);
          ++pos;
        }
      }
      args[i] = strdup(arg_copy);
      duplicated[i] = 1;
    }

    args[++i] = strtok(NULL, " ");
  }
}

void exec_command(char **args) {
  /* Proceseaza input-ul ca pe o comanda normala */

  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    printf(ANSI_COLOR_RED "Error: Unknown command %s\n" ANSI_COLOR_RESET,
           args[0]);
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

void free_args(char **args, int *duplicated) {
  for (int i = 0; args[i] != NULL; i++) {
    if (duplicated[i]) {
      free(args[i]);
    }
    args[i] = NULL;
  }
}

int run_script(char **args) {
  /* Executa script bash */

  if (strlen(args[0]) > 3 &&
      strcmp(args[0] + strlen(args[0]) - 3, ".sh") == 0) {
    if (access(args[0], F_OK) == -1) {
      // Verificam daca fisierul script exista
      fprintf(stderr,
              ANSI_COLOR_RED "Error: File '%s' not found!\n" ANSI_COLOR_RESET,
              args[0]);
      return 1;
    }

    // Initializam argumentele din linia de comanda
    for (int i = 1; args[i] != NULL; i++) {
      char digit[12];
      sprintf(digit, "%d", i);
      set_variable(digit, args[i]);
    }

    FILE *script = fopen(args[0], "r");
    if (!script) {
      perror(ANSI_COLOR_RED "Error: Failed to open script!\n" ANSI_COLOR_RESET);
      return 1;
    }

    // Executam toate liniile
    char line[MAX_INPUT];
    while (fgets(line, sizeof(line), script)) {
      char *arguments[MAX_ARGS];
      int duplicated[MAX_ARGS] = {0};

      line[strlen(line) - 1] = '\0';

      parse_command(line, arguments, duplicated);

      if (strcmp(line, "exit") == 0) {
        free_args(arguments, duplicated);
        return 2;
      }

      if (var_command(arguments)) {
        continue;
      }

      exec_command(arguments);

      free_args(arguments, duplicated);
    }

    // Eliminam variabilelor primite ca argument
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
  int duplicated[MAX_ARGS] = {0};

  while (1) {
    printf(ANSI_COLOR_GREEN "XShell> " ANSI_COLOR_RESET);

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
      free_args(args, duplicated);
      printf("\n");
      break;
    }

    parse_command(input, args, duplicated);

    // Script Bash
    int output = run_script(args);
    if (output == 1) {
      continue;
    } else if (output == 2) {
      break;
    }

    // Comanda specifica variabilelor
    if (var_command(args)) {
      continue;
    }

    // Comanda Bash
    exec_command(args);

    free_args(args, duplicated);
  }

  return EXIT_SUCCESS;
}
