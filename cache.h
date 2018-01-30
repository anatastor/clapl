
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

    cache_entry *artists; // array of artists
    cache_entry *album; // array of albums
    cache_entry *tracks; // array of tracks

    int nartists;   // number of artists loaded
    int nalbum;     // number of albums loaded
    int ntracks;    // number of tracks loaded

    int selectedArtist;
    int selectedAlbum;
    int selectedTrack;

    cache_entry currentTrack; // track which is currently playing

    char *sorting; // options defining the sorting behaviour
    char *lyrics_path; // path to the lyrics
} cache;

/*
 * initialize and load the cache
 * param[in] pointer to the sqlite3 database
 * returns pointer to the initialized cache or NULL on failure
 */
cache *cache_load (sqlite3 *db);

/*
 * reloads the cache
 */
void cache_reload (cache **c);

void cache_entry_load_artists (cache *c); // load artists
void cache_entry_load_album (cache *c); // load albums
void cache_entry_load_tracks (cache *c); // load tracks
char *cache_load_filepath (cache *c); // get path to the selected track

void cache_remove_album (cache *c, int id); // remove the album with the given id
void cache_remove_track (cache *c, int id); // remove the track with the given id

void cache_close (cache *c); // close cache and free memory

#endif
