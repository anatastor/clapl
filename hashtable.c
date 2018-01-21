
#include "hashtable.h"


char *strcup (const char *const s)
{
    char *d = malloc(strlen(s) + 1);
    if (d == NULL)
        return NULL;

    strcpy (d, s);
    return d;
}


hashtable *ht_create (const int size)
{
    hashtable *ht = malloc(sizeof(hashtable));
    if (!ht)
        return NULL;
    if (!ht_init(ht, size))
        return NULL;
    else
        return ht;
}

int ht_init (hashtable *const ht, const int size)
{
    ht->size = size;
    ht->entries = NULL;
    ht->entries = malloc(sizeof(entry*) * size);
    if (! ht->entries)
        exit(1);

    for (int i = 0; i < size; i++)
        ht->entries[i] = NULL;

    return 1; // success
}


int ht_hash (hashtable *const ht, char *const key)
{
    unsigned long int hashval = 0;
    int i = 0;

    while (hashval < ULONG_MAX && i < strlen(key) && key[i] != '\0')
    {
        hashval = hashval << 8;
        hashval += key[i];
        i++;
    }

    return hashval % ht->size;
}


entry *ht_pair (char *const key, char *const value)
{
    entry *e = malloc(sizeof(entry));
    if (! e)
        exit(1);
    
    if ((e->key = strcup(key)) == NULL)
        return NULL;

    if ((e->value = strcup(value)) == NULL)
        return NULL;

    e->next = NULL;
    
    return e;
}


int ht_set (hashtable *const ht, char *const key, char *const value)
{
    int h = ht_hash(ht, key);

    if (!ht->entries[h])
    {
        ht->entries[h] = ht_pair(key, value);
        return 1;
    }
    
    entry *next = ht->entries[h];
    while (next && next->next && next->key && strcmp(next->key, key) != 0)
        next = next->next;
    
    if (next)
        next->next = ht_pair(key, value);
    else
        next = ht_pair(key, value);

    return 1;
}


char *ht_get (hashtable *const ht, char *const key)
{
    int h = ht_hash(ht, key);
    entry *e = ht->entries[h];
    if (! e)
        return NULL;

    while (e && e->next && e->key && strcmp(e->key, key) != 0)
        e = e->next;
    
    if (strcmp(e->key, key) == 0)
        return e->value;
    else
        return NULL;
}    
