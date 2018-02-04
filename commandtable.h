
#ifndef _COMMANDTABLE_H_
#define _COMMANDTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>


typedef union
{
    int i;
    float f;
    char c;
    char *s;
    void *v;
} cmd_arg;


typedef struct
{
    char *name;
    int (*func)(cmd_arg *args);
    char *args;
    char *doc;
} cmd;


typedef struct cmd_table_entry_p
{
    char *key;
    cmd *value;
    struct cmd_table_entry_p *next;
} cmd_table_entry;


typedef struct
{
    int size;
    cmd_table_entry **table;
} cmd_table;


cmd_table *cmd_table_create (const int size);

int cmd_table_init (cmd_table *commands, const int size);

int cmd_table_hash (const cmd_table *commands, const char *key);

cmd_table_entry *cmd_table_pair (char *key, cmd *value);

int cmd_table_set (const cmd_table *commands, char *key, cmd *value);

cmd *cmd_table_get (const cmd_table *commands, const char *key);




#endif
