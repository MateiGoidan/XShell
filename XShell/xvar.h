/* Structura pentru a defini/redefini variabile shell */

#ifndef XVAR
#define XVAR

#define MAX_VARS 100

typedef struct{
  char *name;
  char *value;
} XVar;

// Metode 
void set_variable(char *name, char *value);
char *get_value(char *name);
void unset_variable(char *name);

#endif // !XVAR
