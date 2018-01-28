
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

    int nartists;   // number of artists loaded
    int nalbum;     // number of albums loaded
    int ntracks;    // number of tracks loaded

    int selectedArtist;
    int selectedAlbum;
    int selectedTrack;

    cache_entry currentTrack; // track which is currently playing

    char *sorting;
    char *lyrics_path;
} cache;


cache *cache_load (sqlite3 *db);
// loading the cache 
void cache_reload (cache **c);

void cache_entry_load_artists (cache *c);
void cache_entry_load_album (cache *c);
void cache_entry_load_tracks (cache *c);
char *cache_load_filepath (cache *c);

void cache_remove_album (cache *c, int id);
void cache_remove_track (cache *c, int id);

void cache_close (cache *c);

#endif
