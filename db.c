
#include "db.h"


static int callback (void *data, int argc, char **argv, char **azColName)
{
    // callback function fpr sqlite3 calls
    int *int_data = (int*) data;
    *int_data = atoi(argv[0]); // storing the id of the entry in data
    return 0;
}


sqlite3 *db_init (void)
{
    sqlite3 *db = NULL;
    if (sqlite3_open(DB_NAME, &db)) // open the database
        logcmd(LOG_ERROR_MALLOC, "db: db_init: could not malloc db");

    // create tables if they do not exist
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS \
                artist(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \
                name TEXT NOT NULL);", NULL, 0, NULL);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS \
            album(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \
                name TEXT NOT NULL, \
                artist_id INTEGER NOT NULL);", NULL, 0, NULL);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS  \
            track(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \
                title TEXT NOT NULL, \
                number INTEGER, \
                total_number INTEGER, \
                artist_id INTEGER NOT NULL, \
                album_id INTEGER NOT NULL, \
                path TEXT NOT NULL);", NULL, 0, NULL);
    
    // create 'Playlists' artist if it does not exist
    struct entry e;
    e.artist = "Playlists";
    db_add_artist(db, &e);
    
    logcmd(LOG_DMSG, "db: db_init: db initialized");
    return db;
}


int is_supported_audio_file (const char *file)
{   
    // check if the given file is a supported audio file
    char *end = strrchr(file, '.');
    if (!end)
    {
        logcmd(LOG_DMSG, "db: is_supported_audio_file: could not malloc end");
        return 0;
    }

    const char *supported[] = {
        ".ogg",
        ".mp3"
    };
    int size_supported = 2;

    for (int i = 0; i < size_supported; i++)
    {
        if (strcmp(end, supported[i]) == 0)
            return 1;
    }

    return 0;
}


int db_add_dir (sqlite3 *db, const char *path)
{   
    if (*path == '~')
        path += 2;
    
    // check if path is a file or directory
    struct stat sb;
    stat(path, &sb);
    if (S_ISREG(sb.st_mode))
    {
        // path is a file -> call db_add_file
        db_add_file (db, path);
        return 1;
    }

    DIR *dir = NULL;
    struct dirent *ent = NULL;

    if ((dir = opendir(path))) // open the directory
    {
        if (ent)
        {
            free(ent);
            ent = NULL;
        }
        while ((ent = readdir(dir))) // read files in directory
        {
            if (ent->d_type == DT_DIR) // an other directory
            {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) // do not search recursiv in '.' and '..'
                    continue;

                int size = strlen(path) + strlen(ent->d_name) + 2;
                char *subdir = malloc(sizeof(char) * size);
                if (!subdir)
                    exit(1);

                snprintf(subdir, size, "%s%s/", path, ent->d_name);
                db_add_dir(db, subdir);
                free(subdir);
            }
            else if (ent->d_type == DT_REG)
            {
                if (is_supported_audio_file(ent->d_name))
                {
                    int size = strlen(path) + strlen(ent->d_name) + 1;
                    char *file = malloc(sizeof(char) * size);
                    if (!file)
                        exit(1);

                    snprintf(file, size, "%s%s", path, ent->d_name);
                    int result = db_add_file(db, file);

                    free (file);
                }
            }
        }

    }
    else
        logcmd(LOG_MSG, "could not open directory: %s", path);
    
    closedir(dir);

    return 1;
}


int db_check_artist (sqlite3 *db, const struct entry *e)
{
    /*
     * check if the artist already exists in the database
     *  yes -> return the id
     *  no -> return 0
     */
    logcmd(LOG_DMSG, "checking artist: %s", e->artist);
    char *err = NULL;
    int id = -1;

    int size = snprintf(NULL, 0, CHECK_ARTIST, e->artist);
    char *sql = malloc(sizeof(char) * size);
    if (! sql)
        logcmd(LOG_ERROR_MALLOC, "db: db_check_artist: could not allocate memory for sql");
    snprintf(sql, size, CHECK_ARTIST, e->artist);
    if (sqlite3_exec(db, sql, callback, &id, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "SQL ERROR: %i", err);

    free(sql);
    free(err);

    if (id >= 0)
        return id;

    return 0;
}


int db_check_album (sqlite3 *db, const struct entry *e)
{
    /*
     * check if the album already exists in the database
     *  yes -> return id
     *  no -> return 0
     */
    logcmd(LOG_DMSG, "checking album: %s", e->album);
    char *err = NULL;
    int id = -1;

    int size = snprintf(NULL, 0, CHECK_ALBUM, e->album, e->artist_id);
    char *sql = malloc(sizeof(char) * size);
    if (! sql)
        logcmd(LOG_ERROR_MALLOC, "db: db_check_album: could not allocate memory for sql");
    snprintf(sql, size, CHECK_ALBUM, e->album, e->artist_id);
    if (sqlite3_exec(db, sql, callback, &id, &err) != SQLITE_OK)
        logcmd(LOG_ERROR, "SQL ERROR: %i", err);

    free(sql);
    free(err);

    if (id >= 0)
        return id;

    return 0;
}


int db_check_track (sqlite3 *db, const struct entry *e)
{
    /*
     * check if the title already exists in the database
     *  yes -> return id
     *  no -> return 0
     */
    logcmd(LOG_DMSG, "checking track: %s", e->title);
    char *err = NULL;
    int id = -1;

    int size = snprintf(NULL, 0, CHECK_TRACK, e->title, e->number, e->totalnumber, e->artist_id, e->album_id);
    char *sql = malloc(sizeof(char) * size);
    if (! sql)
        logcmd(LOG_ERROR_MALLOC, "db: db_check_track: unable to allocate memory for sql");
    snprintf(sql, size, CHECK_TRACK, e->title, e->number, e->totalnumber, e->artist_id, e->album_id);
    if (sqlite3_exec(db, sql, callback, &id, &err) != SQLITE_OK)
    {
        logcmd(LOG_ERROR, "db: db_check_track: SQL ERROR: %i", err);
    }

    free(sql);
    free(err);

    if (id >= 0)
        return -2; // exists already in the database

    return 0;
}


int db_add_artist (sqlite3 *db, struct entry *e)
{
    // add the artist to the database
    logcmd(LOG_DMSG, "adding artist: %s", e->artist);
    int artist_id;

    if ((artist_id = db_check_artist(db, e)))
    {
        logcmd(LOG_DMSG, "artist exists");
        e->artist_id = artist_id;
        return 0;
    }
    else
    {
        logcmd(LOG_DMSG, "artist does noe exist yet: %s", e->artist);
        char *err = NULL;
        int size = snprintf(NULL, 0, INSERT_ARTIST, e->artist);
        char *sql = malloc(sizeof(char) * size);
        if (! sql)
            logcmd(LOG_ERROR_MALLOC, "db: db_add_artist: unable to allocate memory for sql");
        snprintf(sql, size, INSERT_ARTIST, e->artist);
        if (sqlite3_exec(db, sql, NULL, 0, &err) != SQLITE_OK)
            logcmd(LOG_ERROR, "SQL ERROR: %i", err);

        free(sql);
        free(err);

        e->artist_id = sqlite3_last_insert_rowid(db);
        return 1;
    }

    return 0;
}


int db_add_album (sqlite3 *db, struct entry *e)
{
    // add the album to the database
    logcmd(LOG_DMSG, "adding album: %s", e->album);
    int album_id;

    if ((album_id = db_check_album(db, e)))
    {
        logcmd(LOG_DMSG, "album exists");
        e->album_id = album_id;
        return 0;
    }
    else
    {
        logcmd(LOG_DMSG, "album does not exist yet: %s", e->album);
        char *err = NULL;
        int size = snprintf(NULL, 0, INSERT_ALBUM, e->album, e->artist_id);
        char *sql = malloc(sizeof(char) * size);
        if (! sql)
            logcmd(LOG_ERROR_MALLOC, "db: db_add_album: unable to allocate memory for sql");
        snprintf(sql, size, INSERT_ALBUM, e->album, e->artist_id);
        if (sqlite3_exec(db, sql, NULL, 0, &err) != SQLITE_OK)
            logcmd(LOG_ERROR, "SQL ERROR: %i", err);

        free(sql);
        free(err);

        e->album_id = sqlite3_last_insert_rowid(db);
        return 1;
    }

    return 0;
}


int db_add_track (sqlite3 *db, struct entry *e)
{
    // add the track to the database
    logcmd(LOG_DMSG, "adding track: %s", e->title);
    int id;

    if (db_check_track(db, e) == -2)
    {
        logcmd(LOG_MSG, "title does already exist: %s", e->title);
        //return -2;
        return 0;
    }
    else
    {
        logcmd(LOG_DMSG, "title does not exist yet: %s", e->title);
        char *err = NULL;
        int size = snprintf(NULL, 0, INSERT_TRACK, e->title, e->number, e->totalnumber, e->artist_id, e->album_id, e->file);
        char *sql = malloc(sizeof(char) * size);
        if (! sql)
            logcmd(LOG_ERROR_MALLOC, "db: db_add_track: unable to allocate memory for sql");
        snprintf(sql, size, INSERT_TRACK, e->title, e->number, e->totalnumber, e->artist_id, e->album_id, e->file);
        if (sqlite3_exec(db, sql, NULL, 0, &err) != SQLITE_OK)
            logcmd(LOG_ERROR, "SQL ERROR: %i", err);

        free(sql);
        free(err);

        //return sqlite3_last_insert_rowid(db);
        return 1;
    }

    return 0;
}



int db_add_file (sqlite3 *db, const char *file)
{
    // add a file to the database
    logcmd(LOG_DMSG, "db: db_add_file: adding a file into the database");
    struct entry *e = db_load_entry_from_file(file);
    if (! e)
        return 0; // file was not loaded
    
    db_add_artist(db, e);
    db_add_album(db, e);
    db_add_track(db, e);

    if (e->artist)
        free(e->artist);
    if (e->album)
        free(e->album);
    if (e->title)
        free(e->title);
    if (e->file)
        free(e->file);
    free(e);
    return 1; // file was added to the database
}


char *load_entry_string (const char *value)
{   
    //logcmd(LOG_DMSG, "tag->value: %s", value);
    // copys the content of "value" into a new string
    char *ptr = malloc(sizeof(char) * (strlen(value) + 1));
    strcpy(ptr, value);
    return ptr;
}


struct entry *db_load_entry_from_file (const char *file)
{
    // loads the metadata from the file into the struct
    logcmd(LOG_DMSG, "loading file: %s", file);
    struct entry *e = malloc(sizeof(struct entry));
    if (! e)
        logcmd(LOG_ERROR_MALLOC, "db: db_load_entry_from_file: unable to allocate memory for e");

    // open the file
    AVFormatContext *fctx = NULL;
    fctx = avformat_alloc_context();
    if (! fctx)
        logcmd(LOG_ERROR_MALLOC, "db: db_load_entry_from_file: could not allocate memory for fctx");

    if (avformat_open_input(&fctx, file, NULL, NULL) < 0)
    {
        logcmd(LOG_MSG, "db: db_load_entry_from_file: could not open file (file not added): %s", file);
        return NULL;
    }

    if (avformat_find_stream_info(fctx, NULL) < 0)
    {
        logcmd(LOG_MSG, "db: db_load_entry_from_file: could not find file info (file not added): %s", file);
        return NULL;
    }


    // open the metadata
    AVDictionary *m;
    if (! (m = fctx->metadata))
        m = fctx->streams[0]->metadata;
    if (! m)
    {
        logcmd(LOG_MSG, "db: db_load_entry_from_file: no metadata found (file not added");
        return NULL;
    }

    // initialize the entry struct
    e->artist = NULL;
    e->album = NULL;
    e->title = NULL;
    e->totalnumber = 0;
    e->totalnumber = 0;
    e->file = NULL;


    // loading the metadata into the struct entry
    AVDictionaryEntry *tag;
    if (tag = av_dict_get(m, "ARTIST", NULL, 0))
        e->artist = load_entry_string(tag->value);
    else if (tag = av_dict_get(m, "album_artist", NULL, 0))
        e->artist = load_entry_string(tag->value);
        //sometimes the tag 'artist' does not exist use 'album_artist' instead

    if (tag = av_dict_get(m, "ALBUM", NULL, 0)) // load artist
        e->album = load_entry_string(tag->value);
    if (tag = av_dict_get(m, "TITLE", NULL, 0)) // load album
        e->title = load_entry_string(tag->value);
    if (tag = av_dict_get(m, "track", NULL, 0)) // load track number
        e->number = atoi(tag->value);
    if (tag = av_dict_get(m, "TRACKTOTAL", NULL, 0)) // load total number of tracks
        e->totalnumber = atoi(tag->value);

    e->file = load_entry_string(file); // store path to the file

    // debugging output
    logcmd(LOG_DMSG, "\nArtist: %s\nAlbum: %s\nTitle: %s\nPath: %s", e->artist, e->album, e->title, e->file);

    avformat_close_input(&fctx);

    return e;
}


void db_close (sqlite3* db)
{
    sqlite3_close(db);
}

