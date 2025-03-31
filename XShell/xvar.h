/* Structura pentru a defini/redefini variabile shell */

#ifndef XVAR
#define XVAR

#define MAX_VARS 100

typedef struct{
  char *name;
  char *value;
} XVar;

// Metode 
int set_variable(char *name, char *value);
char *get_value(char *name);
int unset_variable(char *name);
void free_variables();

#endif // !XVAR
