
#include "cache.h"


char *copy_string (const char *value)
{
    char *ptr = malloc(sizeof(char) * (strlen(value) + 1));
    strcpy(ptr, value);
    return ptr;
}


static int size_callback (void *data, int argc, char **argv, char **azColName)
{
    int *int_data = (int*) data;
    *int_data = atoi(argv[0]);
    return 0;
}


cache *cache_load (sqlite3 *db)
{
    cache *c = malloc(sizeof(cache));

    c->artists = NULL;
    c->album = NULL;
    c->tracks = NULL;
    c->lyrics_path = NULL;

    c->db = db;
    c->artists = cache_entry_load_artists(c->db, &c->nartists);

    return c;
}


cache_entry *cache_entry_load_artists (sqlite3 *db, int *nartists)
{
    char *err = NULL;
    if (sqlite3_exec(db, "SELECT COUNT(*) FROM artist;", size_callback, nartists, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "cache: cache_load_artists: SQL ERROR: %i", err);
    
    if (*nartists == 0)
    {
        logcmd(LOG_DMSG, "cache: cache_load_artists: database is emptry");
        return NULL;
    }
    
    cache_entry *artists = malloc(sizeof(cache_entry) * *nartists);
    if (! artists)
        logcmd(LOG_ERROR_MALLOC, "cache: cache_load_artists: unable to allocate memory for artists");

    sqlite3_stmt *res;
    const char *tail = NULL;
    int error;
    error = sqlite3_prepare_v2(db, "SELECT name, id FROM artist ORDER BY name;", 1000, &res, &tail);

    int count = 0;
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        artists[count].name = copy_string(sqlite3_column_text(res, 0));
        artists[count].id = atoi(sqlite3_column_text(res, 1));
        count++;
    }

    return artists;
}


cache_entry *cache_entry_load_album (sqlite3* db, int *nalbum, int artist_id)
{   
    char *err = NULL;

    int size = snprintf(NULL, 0, "SELECT COUNT(*) FROM album WHERE artist_id=%i;", artist_id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "SELECT COUNT(*) FROM album WHERE artist_id=%i;", artist_id);
    
    if (sqlite3_exec(db, sql, size_callback, nalbum, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "cache: cache_load_album: SQL ERROR: %i", err);

    cache_entry *album = malloc(sizeof(cache_entry) * *nalbum);
    if (! album)
        logcmd(LOG_ERROR_MALLOC, "cache: cache_entry_load_album: unalbe to allocate memory for album");

    sqlite3_stmt *res;
    const char *tail = NULL;
    
    size = snprintf(NULL, 0, "SELECT name, id FROM album WHERE artist_id=%i ORDER BY name;", artist_id);
    sql = realloc(sql, sizeof(char) * size);
    snprintf(sql, size, "SELECT name, id FROM album WHERE artist_id=%i ORDER BY name;", artist_id);
    int error = sqlite3_prepare_v2(db, sql, 1000, &res, &tail);

    int count = 0;
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        album[count].name = copy_string(sqlite3_column_text(res, 0));
        album[count].id = atoi(sqlite3_column_text(res, 1));
        count++;
    }
    
    free(sql);
    return album;
}


cache_entry *cache_entry_load_track (sqlite3 *db, int *ntracks, int album_id, int artist_id)
{
    char *err = NULL;
    
    int size = snprintf(NULL, 0, "SELECT COUNT(*) FROM track WHERE album_id=%i AND artist_id=%i;", album_id, artist_id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "SELECT COUNT(*) FROM track WHERE album_id=%i AND artist_id=%i;", album_id, artist_id);

    if (sqlite3_exec(db, sql, size_callback, ntracks, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "cache: cache_entry_load_track: SQL ERROR: %i", err);

    cache_entry *tracks = malloc(sizeof(cache_entry) * *ntracks);
    if (! tracks)
        logcmd(LOG_ERROR_MALLOC, "cache: cache_entry_load_track: unable to allocate memory for tracks");

    sqlite3_stmt *res;
    const char *tail = NULL;

    size = snprintf(NULL, 0, "SELECT title, id FROM track WHERE album_id=%i AND artist_id=%i ORDER BY number;", album_id, artist_id);
    sql = realloc(sql, sizeof(char) * size);
    snprintf(sql, size, "SELECT title, id FROM track WHERE album_id=%i AND artist_id=%i ORDER BY number;", album_id, artist_id);
    
    int error = sqlite3_prepare_v2(db, sql, 1000, &res, &tail);
    
    int count = 0;
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        tracks[count].name = copy_string(sqlite3_column_text(res, 0));
        tracks[count].id = atoi(sqlite3_column_text(res, 1));
        count++;
    }

    free(sql);
    return tracks;

}


cache_entry *cache_enty_load_album (sqlite3 *db, int *nalbum)
{   
    char *err;
    return NULL;
}


char *cache_load_filepath (sqlite3 *db, int track_id)
{   
    char *err = NULL;
    int size = snprintf(NULL, 0, "SELECT path FROM track WHERE id=%i;", track_id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "SELECT path FROM track WHERE id=%i;", track_id);
    
    char *string;
    sqlite3_stmt *res;
    const char *tail = NULL;
    int error = sqlite3_prepare_v2(db, sql, 1000, &res, &tail);
    if (sqlite3_step(res) == SQLITE_ROW)
    {
        string = copy_string(sqlite3_column_text(res, 0));
    }

    free(sql);
    return string;
}


void cache_close (cache *c)
{
    c->currentTrack.name = NULL;
    c->currentTrack.id = 0;
    free(c->artists);
    free(c->album);
    free(c->tracks);
    free(c);
}
