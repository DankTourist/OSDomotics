#include "common.h"
#include <string.h>
#include <stdlib.h>

char **tokenise(char *str){
  char *toks[] = (char**) malloc(sizeof(char*) * (MAX_TOKENS + 1));
  toks[0] = strtok(str, " ");
  int i = 1;
  
  do {
    toks[i] = strtok(NULL, " ");
    ++i;
  }
  while(toks[i-1] != NULL && i < MAX_TOKENS);

  if(i == MAX_TOKENS) toks[i] = NULL;
  return toks;
}