/**
 * @file
 * @author Torsten Lehmann
 * @date 07-01-2018
 *
 * simple parser for config files
 */


#ifndef _CONFIGPARSER_H_
#define _CONFIGPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "logger.h"


typedef struct 
{
    FILE *file;
    hashtable *ht;
    char delimiter;
} configparser;


void configparser_init (configparser *const cp, char *const file, const int size, const char delimiter);
void configparser_free (configparser **cp);

void configparser_init_file (configparser *const cp, FILE *file, const int size, const char delimiter);

void configparser_load (configparser *const cp);

char *configparser_split_string (char *const string, const char delimiter);

int configparser_get_bool (configparser *const cp, char *const key);
int configparser_get_int (configparser *const cp, char *const key);
char *configparser_get_string (configparser *const cp, char *const key);

#endif
