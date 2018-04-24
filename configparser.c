
#include "configparser.h"


char *read_line (FILE *const file, const int buffer_size)
{
    char c;
    char *line = malloc(sizeof(char) * buffer_size);

    for (int i = 0; i < buffer_size; i++)
        line[i] = 0;
    
    int pos = 0;
    c = getc(file);
    while(c != '\n' && c != EOF)
    {
        line[pos] = c;
        if (++pos >= buffer_size)
            break;
        c = getc(file);
    }
    

    if (c == EOF && pos == 0)
    {
        free(line);
        return NULL;
    }

    if (c == '\n' && pos == 0) // removes empty lines
    {
        free(line);
        return read_line(file, buffer_size);
    }

    if (line[0] == '#')
    {
        free(line);
        return read_line(file, buffer_size);
    }

    char *ptr = strrchr(line, '#');
    if (ptr)
        *ptr = '\0';

    return line;
}


void configparser_init (configparser *const cp, char *const file, const int size, const char delimiter)
{
    cp->file = fopen(file, "r"); // open file
    if (! cp->file)
        logcmd(LOG_ERROR_MALLOC, "configparser: configparser_init: could not malloc cp->file");

    cp->delimiter = delimiter;
    cp->ht = ht_create(size); // create hashtable
}


configparser *configparser_delete (configparser *cp)
{   
    cp->ht = ht_free(cp->ht);
    free(cp);
    return NULL;
}


void configparser_init_file (configparser *const cp, FILE *file, const int size, const char delimiter)
{
    cp->file = file;
    cp->delimiter = delimiter;
    cp->ht = ht_create(size);
}


void configparser_load (configparser *const cp)
{
    char *line = read_line (cp->file, 255);
    char *value = NULL;
    while (line)
    {
        value = configparser_split_string(line, cp->delimiter);
        ht_set(cp->ht, line, value);
        free(line);
        value = NULL;
        line = read_line(cp->file, 255);
    }
    
    if (line)
        free(line);
}


char *configparser_split_string (char *const string, const char delimiter)
{
    char *new = strchr(string, delimiter);
    if (!new)
        return NULL;
    *new = '\0';
    return ++new;
}


int configparser_get_bool (configparser *const cp, char *const key)
{
    char *value = ht_get(cp->ht, key);
    switch(*value)
    {
        case 't': case 'T':
            return 1;

        case 'f': case 'F':
            return 0;

        case '0':
            return 0;

        case '1':
            return 1;

        default:
            return 0;
    }

    return 0;
}


int configparser_get_int (configparser *const cp, char *const key)
{
    char *value = ht_get(cp->ht, key);
    if (value)
        return (atoi(value));

    return 0;
}


char *configparser_get_string (configparser *const cp, char *const key)
{
    return ht_get(cp->ht, key);
}

