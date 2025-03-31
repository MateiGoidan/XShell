#include "xvar.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

XVar xvars[MAX_VARS]; // Array de variabile shell
int xvar_numb = 0;    // Numarul variabilelor shell

int set_variable(char *name, char *value) {
  /* Seteaza valoare "value" variabilei cu valoare name. Daca variabila "name"
   * nu se afla in array-ul "xvars" atunci definim una nou. Daca deja exista o
   * variabila cu acest nume, atunci o redefinim cu valoarea "value". */

  // Cazul 1: Avem deja o variabila cu numele "name"
  for (int i = 0; i < xvar_numb; i++) {
    if (!strcmp(name, xvars[i].name)) {
      free(xvars[i].value);

      xvars[i].value = strdup(value);
      if (xvars[i].value == NULL) {
        perror("Error: strdup failed (xvars.value 1)!\n");
        return 0;
      }

      setenv(name, value, 1);

      return 1;
    }
  }

  // Cazul 2: Nu avem o variabila cu numele "name"
  if (xvar_numb < MAX_VARS) {
    xvars[xvar_numb].name = strdup(name);
    if (xvars[xvar_numb].name == NULL) {
      perror("Error: strdup failed (xvars.name)!\n");
      return 0;
    }

    xvars[xvar_numb].value = strdup(value);
    if (xvars[xvar_numb].value == NULL) {
      perror("Error: strdup failed (xvars.value)!\n");
      free(xvars[xvar_numb].name);
      return 0;
    }

    setenv(name, value, 0);
    xvar_numb++;

    return  1;
  }
  else {
    perror("Error: Maximum number of variables reached!");
    return 0;
  }
}

char *get_value(char *name) {
  /* Returneaza valoarea variabilei cu numele "name" */

  for (int i = 0; i < xvar_numb; i++) {
    if (!strcmp(name, xvars[i].name)) {
      return xvars[i].value;
    }
  }

  return NULL;
}

int unset_variable(char *name) {
  /* Elimina variabila cu numele "name" */

  for (int i = 0; i < xvar_numb; i++) {
    if (!strcmp(name, xvars[i].name)) {
      // Eliberare memorie
      free(xvars[i].name);
      free(xvars[i].value);

      // Configurare array
      xvars[i] = xvars[--xvar_numb];
      return 1;
    }
  }

  return 0;
}


void free_variables() {
  for (int i = 0; i < xvar_numb; i++) {
    unset_variable(xvars[i].name);
  }

  xvar_numb = 0;
}
