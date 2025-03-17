#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_SIZE 128

int main(int argc, char *argv[]) {
  char input[MAX_SIZE];
  char command[MAX_SIZE];

  while (1) {
    printf("XShell> ");

    // Ignora cazul in care input-ul este gol
    if (fgets(input, MAX_SIZE, stdin) == NULL) {
      continue;
    }

    // Inlatura "\n" de la sfarsitul liniei
    input[strlen(input) - 1] = '\0';

    // Iesire shell
    if (strcmp(input, "exit") == 0) {
      break;
    }
  }

  return EXIT_SUCCESS;
}
