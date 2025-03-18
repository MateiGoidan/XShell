#include "xvar.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

XVar xvars[MAX_VARS]; // Array de variabile shell
int xvar_numb = 0;    // Numarul variabilelor shell

void set_variable(char *name, char *value) {
  /* Seteaza valoare "value" variabilei cu valoare name. Daca variabila "name"
   * nu se afla in array-ul "xvars" atunci definim una nou. Daca deja exista o
   * variabila cu acest nume, "name", atunci o redefinim cu valoarea "value". */

  // Cazul 1: Avem deja o variabila cu numele "name"
  for (int i = 0; i < xvar_numb; i++) {
    if (!strcmp(name, xvars[i].name)) {
      free(xvars[i].value);
      xvars[i].value = strdup(value);
      setenv(name, value, 1);
      return;
    }
  }

  // Cazul 2: Nu avem o variabila cu numele "name"
  if (xvar_numb < MAX_VARS) {
    xvars[xvar_numb].name = strdup(name);
    xvars[xvar_numb++].value = strdup(value);
    setenv(name, value, 0);
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

void unset_variable(char *name) {
  /* Elimina variabila cu numele "name" */

  for (int i = 0; i < xvar_numb; i++) {
    if (!strcmp(name, xvars[i].name)) {
      // Eliberare memorie
      free(xvars[i].name);
      free(xvars[i].value);
      unsetenv(name);
      
      // Configurare array
      xvars[i] = xvars[--xvar_numb];
      return;
    }
  }
}
