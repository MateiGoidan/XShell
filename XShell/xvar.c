#include "xvar.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

XVar xvars[MAX_VARS]; // Shell variable array
int xvar_numb = 0;    // Number of shell variables

int set_variable(char *name, char *value) {
  /* Sets the value "value" to the variable named "name". 
   * If the variable "name" is not found in the "xvars" array, a new one is defined.
   * If a variable with this name already exists, it is redefined with the new value. */

  // Case 1: We already have a variable with the name "name"
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

  // Case 2: We don't have a variable with the name "name"
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
  /* Returns the value of the variable with the name "name" */

  for (int i = 0; i < xvar_numb; i++) {
    if (!strcmp(name, xvars[i].name)) {
      return xvars[i].value;
    }
  }

  return NULL;
}

int unset_variable(char *name) {
  /* Removes the variable with the name "name" */

  for (int i = 0; i < xvar_numb; i++) {
    if (!strcmp(name, xvars[i].name)) {
      // Free memory
      free(xvars[i].name);
      free(xvars[i].value);

      // Adjust the array
      xvars[i] = xvars[--xvar_numb];
      return 1;
    }
  }

  return 0;
}

void free_variables() {
  /* Frees all allocated variables */
  for (int i = 0; i < xvar_numb; i++) {
    unset_variable(xvars[i].name);
  }

  xvar_numb = 0;
}
