
#ifndef _CACHE_H_
#define _CACHE_H_


#include "logger.h"
#include "db.h"


typedef struct cache_entry
{
    char* name;
    int id;
} cache_entry; 


typedef struct cache
{
    sqlite3 *db;

    cache_entry *artists;
    cache_entry *album;
    cache_entry *tracks;

    int nartists;
    int nalbum;
    int ntracks;

    cache_entry currentTrack;
    
    char *lyrics_path;
} cache;


cache *cache_load (sqlite3 *db);
cache_entry *cache_entry_load_artists (sqlite3 *db, int *nartists);
cache_entry *cache_entry_load_album (sqlite3 *db, int *nalbum, int artist_id);
cache_entry *cache_entry_load_track (sqlite3 *db, int *ntacks, int album_id, int artist_id);
char *cache_load_filepath (sqlite3 *db, int track_id);

void cache_close (cache *c);

#endif
