
#ifndef _COMMANDPARSER_H_
#define _COMMANDPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commandtable.h"

/*

typedef union
{
    int i;
    float f;
    char c;
    char *s;
} arg_t;


typedef struct
{
    char *name;
    void (*func)(arg_t *args);
    char *args;
    char *docs;
} cmd_t;
*/


void commandparser_parse (const cmd_table *command_list, char *command);

cmd_arg *cmdparser_get_args (char *command, const char *args_s);




#endif
