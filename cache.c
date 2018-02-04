
#include "cache.h"


char *copy_string (const char *value)
{
    // copies a string into a new space of memory
    char *ptr = malloc(sizeof(char) * (strlen(value) + 1));
    strcpy(ptr, value);
    return ptr;
}


static int size_callback (void *data, int argc, char **argv, char **azColName)
{   
    // callback for sqlite3 methods
    int *int_data = (int*) data;
    *int_data = atoi(argv[0]);
    return 0;
}


cache *cache_load (sqlite3 *db)
{
    cache *c = malloc(sizeof(cache));
    if (!c)
        logcmd(LOG_ERROR_MALLOC, "cache: cache_load: unable to allocate memory for cache");
    
    // initialize values
    c->artists = NULL;
    c->album = NULL;
    c->tracks = NULL;
    c->lyrics_path = NULL;
    c->sorting = "number";
    c->selectedArtist = -1;
    c->selectedAlbum = -1;
    c->selectedTrack = -1;
    c->currentTrack.id = -1;
    c->currentTrack.name = "";

    c->db = db;
    cache_entry_load_artists(c);
    
    logcmd(LOG_DMSG, "cache initialized");
    return c;
}


void cache_reload (cache **c)
{   
    logcmd(LOG_DMSG, "cache_reload: executing");
    cache *cc = *c;
    // copy the path to the lyrics
    int size = snprintf(NULL, 0, cc->lyrics_path);
    char *path = malloc(sizeof(char) * size);
    strcpy(path, cc->lyrics_path);

    cmd_table *ct = cc->commands;
    cc->commands = NULL;
    
    // copy the sorting option
    size = snprintf(NULL, 0, cc->sorting);
    char *sort = malloc(sizeof(char) * size);
    strcpy(sort, cc->sorting);

    struct cache_entry e = cc->currentTrack;
    sqlite3 *db = cc->db;
    cache_close(*c);
    *c = cache_load(db);
    cc = *c;
    cc->commands = ct;
    cc->lyrics_path = path; // put path to lyrics back into the new cache
    cc->sorting = sort; // put sorting option back into the new cache
    cc->currentTrack = e;
}


void cache_entry_load_artists (cache *c)
{
    char *err = NULL;
    // get number of artists in the database
    if (sqlite3_exec(c->db, "SELECT COUNT(*) FROM artist;", size_callback, &c->nartists, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "cache: cache_load_artists: SQL ERROR: %i", err);
    
    if (c->nartists == 0)
    {
        logcmd(LOG_DMSG, "cache: cache_load_artists: database is emptry");
        return;
    }
    
    // create array of artists
    cache_entry *artists = malloc(sizeof(cache_entry) * c->nartists);
    if (! artists)
        logcmd(LOG_ERROR_MALLOC, "cache: cache_load_artists: unable to allocate memory for artists");

    sqlite3_stmt *res;
    const char *tail = NULL;
    int error;
    error = sqlite3_prepare_v2(c->db, "SELECT name, id FROM artist ORDER BY name;", 1000, &res, &tail);
    
    // fill array of artists with values from the database
    int count = 0;
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        artists[count].name = copy_string(sqlite3_column_text(res, 0));
        artists[count].id = atoi(sqlite3_column_text(res, 1));
        count++;
    }
    
    logcmd(LOG_DMSG, "nartists: %i", c->nartists);
    c->artists = artists;
    c->selectedArtist = 0;
}


void cache_entry_load_album (cache *c)
{   
    if (c->album)
    {
        free(c->album);
        c->album = NULL;
    }
    if (c->selectedArtist == -1)
        return;

    char *err = NULL;

    int size = snprintf(NULL, 0, "SELECT COUNT(*) FROM album WHERE artist_id=%i;",
            c->artists[c->selectedArtist].id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "SELECT COUNT(*) FROM album WHERE artist_id=%i;",
            c->artists[c->selectedArtist].id);
    
    // get number of albums of the artist with 'id' in the database
    if (sqlite3_exec(c->db, sql, size_callback, &c->nalbum, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "cache: cache_load_album: SQL ERROR: %i", err);
    
    // allocate memory album array
    cache_entry *album = malloc(sizeof(cache_entry) * c->nalbum);
    if (! album)
        logcmd(LOG_ERROR_MALLOC, "cache: cache_entry_load_album: unalbe to allocate memory for album");

    sqlite3_stmt *res;
    const char *tail = NULL;
    
    size = snprintf(NULL, 0, "SELECT name, id FROM album WHERE artist_id=%i ORDER BY name;",
            c->artists[c->selectedArtist].id);
    sql = realloc(sql, sizeof(char) * size);
    snprintf(sql, size, "SELECT name, id FROM album WHERE artist_id=%i ORDER BY name;",
            c->artists[c->selectedArtist].id);
    int error = sqlite3_prepare_v2(c->db, sql, 1000, &res, &tail);

    // fill album array
    int count = 0;
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        album[count].name = copy_string(sqlite3_column_text(res, 0));
        album[count].id = atoi(sqlite3_column_text(res, 1));
        count++;
    }
    
    free(sql);
    c->album = album;
    c->selectedAlbum = 0;
}


void cache_entry_load_tracks (cache *c)
{
    if (c->tracks)
    {
        free(c->tracks);
        c->tracks = NULL;
    }

    if (c->selectedArtist == -1 || c->selectedAlbum == -1)
        return;

    char *err = NULL;
    
    int size = snprintf(NULL, 0, "SELECT COUNT(*) FROM track WHERE album_id=%i AND artist_id=%i;", 
            c->album[c->selectedAlbum].id,
            c->artists[c->selectedArtist].id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "SELECT COUNT(*) FROM track WHERE album_id=%i AND artist_id=%i;",
            c->album[c->selectedAlbum].id,
            c->artists[c->selectedArtist].id);
    
    // get number of tracks 
    if (sqlite3_exec(c->db, sql, size_callback, &c->ntracks, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "cache: cache_entry_load_track: SQL ERROR: %i", err);

    cache_entry *tracks = malloc(sizeof(cache_entry) * c->ntracks);
    if (! tracks)
        logcmd(LOG_ERROR_MALLOC, "cache: cache_entry_load_track: unable to allocate memory for tracks");

    sqlite3_stmt *res;
    const char *tail = NULL;

    size = snprintf(NULL, 0, "SELECT title, id FROM track WHERE album_id=%i AND artist_id=%i ORDER BY %s;",
            c->album[c->selectedAlbum].id,
            c->artists[c->selectedArtist].id,
            c->sorting);
    sql = realloc(sql, sizeof(char) * size);
    snprintf(sql, size, "SELECT title, id FROM track WHERE album_id=%i AND artist_id=%i ORDER BY %s;",
            c->album[c->selectedAlbum].id,
            c->artists[c->selectedArtist].id,
            c->sorting);
    
    int error = sqlite3_prepare_v2(c->db, sql, 1000, &res, &tail);
    
    // fill track array
    int count = 0;
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        tracks[count].name = copy_string(sqlite3_column_text(res, 0));
        tracks[count].id = atoi(sqlite3_column_text(res, 1));
        count++;
    }

    free(sql);
    c->tracks = tracks;
    c->selectedTrack = 0;
}


char *cache_load_filepath (cache *c)
{
    logcmd(LOG_DMSG, "track_id: %i", c->tracks[c->selectedTrack].id);
    char *err = NULL;
    int size = snprintf(NULL, 0, "SELECT path FROM track WHERE id=%i;", c->tracks[c->selectedTrack].id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "SELECT path FROM track WHERE id=%i;", c->tracks[c->selectedTrack].id);
    
    char *string;
    sqlite3_stmt *res;
    const char *tail = NULL;
    int error = sqlite3_prepare_v2(c->db, sql, 1000, &res, &tail);
    if (sqlite3_step(res) == SQLITE_ROW)
        string = copy_string(sqlite3_column_text(res, 0));

    free(sql);
    return string;
}


void cache_remove_album (cache *c, int id)
{
    for (int i = 0; i < c->ntracks; i++)
        cache_remove_track(c, c->tracks[i].id);

    int size = snprintf(NULL, 0, "DELETE FROM album WHERE id=%i;", id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "DELETE FROM album WHERE id=%i;", id);
    sqlite3_exec(c->db, sql, NULL, 0, NULL);
    free(sql);
    cache_entry_load_album(c);
}


void cache_remove_track (cache *c, int id)
{
    int size = snprintf(NULL, 0, "DELETE FROM track WHERE id=%i;", id);
    char *sql = malloc(sizeof(char) * size);
    snprintf(sql, size, "DELETE FROM track WHERE id=%i;", id);
    sqlite3_exec(c->db, sql, NULL, 0, NULL);
    free(sql);
    cache_entry_load_tracks(c);
}


void cache_close (cache *c)
{
    c->currentTrack.name = NULL;
    c->currentTrack.id = -1;
    // close members of cache
    if (c->artists)
        free(c->artists);
    if (c->album)
        free(c->album);
    if (c->tracks)
        free(c->tracks);
    if (c->lyrics_path)
        free(c->lyrics_path);
    if (c->sorting)
        free(c->sorting);
    if (c->commands)
        free(c->commands);
    free(c);
}
