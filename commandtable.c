
#include "commandtable.h"


cmd_table *cmd_table_create (const int size)
{
    cmd_table *commands = malloc(sizeof(cmd_table));
    if (!commands)
        return NULL;
    
    cmd_table_init(commands, size);

    return commands;
}


int cmd_table_init (cmd_table *commands, const int size)
{
    commands->table = malloc(sizeof(cmd_table_entry) * size);
    if (!commands->table)
        return 0;

    commands->size = size;

    for (int i = 0; i < commands->size; i++)
        commands->table[i] = NULL;

    return 1;
}


void cmd_table_entry_free (cmd_table_entry *e)
{
    if (e->next)
        cmd_table_entry_free(e->next);
    free(e);
}


cmd_table *cmd_table_free (cmd_table *commands)
{   
    if (!commands)
        return NULL;
    
    for (int i = 0; i < commands->size; i++)
    {
        cmd_table_entry *e = commands->table[i];
        if (e)
            cmd_table_entry_free(e);
    }
    free(commands->table);
    free(commands);
    return NULL;
}


int cmd_table_hash (const cmd_table *commands, const char *key)
{
    unsigned long int hashval = 0;
    int i = 0;
    
    while (hashval < ULONG_MAX && i < strlen(key))
    {   
        hashval = hashval << 8;
        hashval += key[i];
        i++;
    }

    return hashval % commands->size;
}


cmd_table_entry *cmd_table_pair (char *key, cmd *value)
{
    cmd_table_entry *e = malloc(sizeof(cmd_table_entry));
    if (!e)
        return NULL;

    e->key = key;
    e->value = value;
    e->next = NULL;

    return e;
}


int cmd_table_set (const cmd_table *commands, char *key, cmd *value)
{
    int h = cmd_table_hash(commands, key);

    if (!commands->table[h])
    {
        commands->table[h] = cmd_table_pair(key, value);
        return 1;
    }

    cmd_table_entry *next = commands->table[h];
    while (next && next->next && next->key && strcmp(next->key, key) != 0)
        next = next->next;

    if (next)
        next->next = cmd_table_pair(key, value);

    return 1;
}


cmd *cmd_table_get (const cmd_table *commands, const char *key)
{
    int h = cmd_table_hash(commands, key);
    cmd_table_entry *e = commands->table[h];
    if (!e)
        return NULL;

    while (e && e->next && e->key && strcmp(e->key, key) != 0)
        e = e->next;

    if (strcmp(e->key, key) == 0)
        return e->value;

    return NULL;
}
