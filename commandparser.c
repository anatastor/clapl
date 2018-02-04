
#include "commandparser.h"


void commandparser_parse (const cmd_table *command_list, char *command)
{
    command[strlen(command) - 1] = '\0';

    char *token = strtok(command, " ");
    cmd *c = cmd_table_get(command_list, token);
    if (!c)
        return;

    cmd_arg *args = cmdparser_get_args(NULL, c->args);
    c->func(args);
}


void cmd_arg_init (cmd_arg *arg)
{
    arg->i = 0;
    arg->f = 0.0f;
    arg->c = 0;
    arg->s = NULL;
    arg->v = NULL;
}


cmd_arg *cmdparser_get_args (char *command, const char *args_s)
{
    int argc = strlen(args_s);
    cmd_arg *args = NULL;
    if (argc)
    {
        args = malloc(sizeof(cmd_arg) * argc);
        for (int i = 0; i < argc; i++)
            cmd_arg_init(&args[i]);
    }

    char *token = strtok(NULL, " ");

    for (int i = 0; i < argc && token; i++)
    {
        switch (args_s[i])
        {
            case 'i':
                args[i].i = atoi(token);
                break;

            case 'f':
                if (scanf(token, "%f", &args[i].f) < 0)
                    return NULL;
                break;

            case 'c':
                args[i].c = token[0];
                break;

            case 's':
                args[i].s = token;
                break;

            case 'v':
                continue;
                break;

            default:
                continue;
                break;
        }
        token = strtok(NULL, " ");
    }

    return args;
}
