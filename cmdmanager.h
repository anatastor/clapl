/*
 * This file is part of commandManager.
 *
 * It is a free Manager for Command written in C.
 * It allows the managing and execution of functions by commands.
 */
/**
 * \file cmdinterpreter.h
 * \author Torsten Lehmann
 * \date 2018-05-20
 * 
 * \brief interpreter for custom command. Might be used in CL-applications.
 *
 */

#ifndef _CMDMANAGER_H_
#define _CMDMANAGER_H_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIM   " "


typedef union
{
    int i;
    double d;
    char c;
    char *s;
    void *v;
} cmd_arg;

/**
 * \brief structure containing the details of a command
 */
typedef struct
{
    char dynamic; /**< boolean describing wether the command has been allocated on the stack (1) or on the heap (0) */
    char *name; /**< name of the command */
    int (*func)(cmd_arg *args); /**< function-pointer to the function the command should call */
    char *args; /**< list of arguments the command requires */ 
    char *doc; /**< string containing the documentation of the command (might be used for a help function) */
} cmd;


/**
 * \brief an entry in the hashtable used to store the commands
 */
struct cmd_table_entry
{
    cmd *command; /**< command actual value in form of a pointer to the command */
    struct cmd_table_entry *next; /**< pointer to the next entry (requiered in case of collision */
} cmd_table_entry;

/**
 * \brief The struct for storing the commands. It works as a hashtable.
 */
typedef struct
{
    int size; /**< size of the hashtable */
    struct cmd_table_entry **entries; /**< array of pointers to the commands */
} cmd_table;


/**
 * \brief creates a hashtable and initializes it
 * \param size size of the hashtable
 * \return pointer to the hashtable, NULL on failure
 */
cmd_table *cmd_table_create (const int size);

/**
 * \brief initializes a hashtable, exits program on failure
 * \param pointer to the hashtable
 */
void cmd_table_init (cmd_table *table, const int size);

/**
 * \brief frees a previous created hashtable
 * \param table pointer to pointer to the hashtable, the table will be set to NULL
 */
void cmd_table_free (cmd_table **table);

/**
 * \brief hashing function
 * \param table pointer to the hashtable
 * \param key key to be hashed
 * \return hashvalue or -1 on failure
 */
int cmd_table_hash (cmd_table *table, char *key);

/**
 * \brief function to insert a command into the hashtable
 * \param table pointer to the hashtable
 * \param command command to be inserted in the hashtable
 * \return 1 on success, 0 on failure
 */
int cmd_table_set (cmd_table *table, cmd *command);

/**
 * \brief function for receiving a command from the hashtable
 * \param table pointer to the hashtable
 * \param key key/name of the command
 * \return pointer to the command on success or NULL on failure
 */
cmd *cmd_table_get (cmd_table *table, char *key);


/**
 * \brief function to get the arguments from the input
 * \param args_s string containing details about the parameters (i for int, d for double, c for char, s for string, v for void-pointer, ...)
 * \return pointer to an array of arguments or NULL on failure
 */
cmd_arg *cmd_parser_get_args (const char *args_s);

/**
 * \brief executes a command
 * \param input string containing the command and its parameters
 * \returns the value of the command-function or -1 on failure
 */
int cmd_execute (cmd_table *table, char *icmd); // icmd = input command string





#endif /* _CMDMANAGER_H_ */
