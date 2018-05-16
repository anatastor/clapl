
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct ht_entry
{   
    char *key;
    char *value;
    struct ht_entry *next;
} ht_entry;


typedef struct
{
    int size;
    ht_entry **entries;
} hashtable;


hashtable *ht_create (const int size);
/**
 * @func
 * @param[in] size of the hashtable
 *
 * \return pointer to the hashtable, NULL on failure
 */

int ht_init (hashtable *const ht, const int size);
/**
 * @func
 * @param[in] pointer to the hashtable
 * @param[in] size of the hashtable
 *
 * \return 1 on success, 0 on failure
 */

void ht_free (hashtable **ht);
/**
 * @func
 * frees the hashtable and sets it's pointer to NULL
 *
 * @param[in] pointer to the pointer to a hashtable
 */


int ht_hash (hashtable *const ht, char *const key);
/**
 * @func
 * @param[in] pointer to the hashtable
 * @param[in] key as a string
 * 
 * \return hashvalue
 */

int ht_set (hashtable *const ht, char *const key, char *const value);
/**
 * @func
 * @param[in] pointer to the hashtable
 * @param[in] key as a string
 * @param[in] value as a string
 *
 * \return 1 on success, 0 on failure
 */

char *ht_get (hashtable *const ht, char *const key);
/**
 * @func
 * @param[in] pointer to the haschtable
 * @param[in] key as a string
 *
 * \return value on success, NULL on failure
 */


#endif
