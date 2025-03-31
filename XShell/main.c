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

void parse_command(char *input, char **args) {
  /* Imparte input-ul in argumente si expandam variabile */
  int index = 0;
  char arg_copy[MAX_INPUT] = "";

  while (*input) {
    // Final input
    if (*input == '\0') {
      ++index;
      break;
    }

    // Ignoram spatiile dintre argumente
    while (*input == ' ') {
      ++input;
    }

    arg_copy[0] = '\0';

    // Parcurgem argumentul
    while (*input && *input != ' ') {
      if (*input == '$') {
        // Gasim o variabila, care incepe cu '$'
        char name[100] = "";
        char non_var_name[100] = "";
        strncat(non_var_name, input, 1);
        ++input;

        if (*input == '$') {
          strcat(name, "$");
        } else {

          int j = 0;
          while (*input && (isalnum(*input) || *input == '_')) {
            // Citim numele complet al variabilei
            name[j++] = *input++;
          }
          name[j] = '\0';
        }

        // Inlocuim cu valoarea variabilei daca exista
        char *value = get_value(name);
        if (value) {
          strcat(arg_copy, value);
        }

        // "$$" caz
        if (!strcmp(name, "$")) {
          ++input;
        }
      } else {
        // Nu gasim o variabila
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
  /* Proceseaza input-ul ca pe o comanda normala */

  // Sarim peste declararea de variabile
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
    printf(ANSI_COLOR_RED "Error: Unknown command %s\n" ANSI_COLOR_RESET,
           args[i]);
    exit(1);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0); // Salvam output-ul copilului

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
  /* Executa comenzi specifice varibilelor */

  // Unset variabile
  if (!strcmp(args[0], "unset") && args[1]) {
    for (int i = 1; args[i] != NULL; i++) {
      if (!unset_variable(args[i]) && visible) {
        printf(ANSI_COLOR_YELLOW
               "Warning: Variable '%s' not found.\n" ANSI_COLOR_RESET,
               args[i]);
      }
    }

    return;
  }

  // Setam variabile
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
        printf(ANSI_COLOR_SUCCESS
               "Notice: Variable '%s' alocated.\n" ANSI_COLOR_RESET,
               name);
      }
    }
  }
}

void free_args(char **args) {
  /* Eliberam memoria alocata argumentelor */
  for (int i = 0; args[i] != NULL; i++) {
    free(args[i]);
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

    // Verificam daca nu am ajuns la numarul maxim de apeluri recursive
    if (script_depth >= MAX_SCRIPT_DEPTH) {
      fprintf(
          stderr, ANSI_COLOR_RED
          "Error: Maximum script recursion depth reached!\n" ANSI_COLOR_RESET);
      return 1;
    }

    // Initializam argumentele din linia de comanda
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

    // Executam toate liniile
    char line[MAX_INPUT];
    while (fgets(line, sizeof(line), script)) {
      char *arguments[MAX_ARGS];

      // Sarim peste comentarii
      if (line[0] == '#') {
        continue;
      }

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

        // Comanda
        int code = exec_command(arguments);
        if (code == 0) {
          free_args(arguments);
          continue;
        }
      }

      var_command(arguments, 0);

      free_args(arguments);
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

  // Setam variabila PID
  char pid_str[20];
  pid_t pid = getpid();
  snprintf(pid_str, sizeof(pid_str), "%d", pid);
  set_variable("$", pid_str);
  // printf("PID as string: %s\n", get_value("$"));

  while (1) {
    printf(ANSI_COLOR_GREEN "XShell> " ANSI_COLOR_RESET);

    // Ignora cazul in care input-ul este gol
    if (fgets(input, MAX_INPUT, stdin) == NULL) {
      continue;
    }

    if (!strcmp(input, "\n")) {
      continue;
    }

    // Inlatura "\n" de la sfarsitul liniei
    input[strlen(input) - 1] = '\0';

    parse_command(input, args);

    /* for (int i = 0; args[i] != NULL; i++) {
      printf("arg[%d] = %s\n", i, args[i]);
    } */

    // Ignora cazul in care input-ul este doar spatii goale
    if (!strcmp(args[0], "\0")) {
      continue;
    }

    // Iesire shell
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

      // Comanda
      int code = exec_command(args);
      if (code == 0) {
        free_args(args);
        continue;
      }
    }

    // Variabile
    var_command(args, 1);

    free_args(args);
  }

  return EXIT_SUCCESS;
}
