#ifndef _DB_H_
#define _DB_H_

#include <string.h>
#include <dirent.h>
#include <sqlite3.h>
#include <sys/stat.h>

#include <libavformat/avformat.h>

#include "logger.h"


#define DB_NAME ".config/clapl/music_db"


#define CHECK_ARTIST    "SELECT id FROM artist WHERE name=\"%s\";"
#define CHECK_ALBUM     "SELECT id FROM album WHERE name=\"%s\" AND artist_id=%i;"
#define CHECK_TRACK     "SELECT id FROM track WHERE title=\"%s\" AND number=%i AND total_number=%i AND artist_id=%i AND album_id=%i AND path=\"%s\";"

#define INSERT_ARTIST   "INSERT INTO artist(id, name) VALUES(NULL, \"%s\");"
#define INSERT_ALBUM    "INSERT INTO album(id, name, artist_id) VALUES(NULL, \"%s\", %i);"
#define INSERT_TRACK    "INSERT INTO track(id, title, number, total_number, artist_id, album_id, path) VALUES(NULL, \"%s\", %i, %i, %i, %i, \"%s\");"




struct entry
{
    char *artist;
    char *album;
    char *title;
    char *file;
    int number;
    int totalnumber;

    int artist_id;
    int album_id;
} entry;


sqlite3 *db_init (void);

int db_add_artist (sqlite3 *db, struct entry *e);
/*
 * add artist to database
 * return 1 if artists was added, 0 if it wasn't added
 */

int db_add_album (sqlite3 *db, struct entry *e);
/**
 * add a album to the database
 * return 1 on success, 0 if it wasn't added
 */

int db_add_track (sqlite3 *db, struct entry *e);
/*
 * add track into the database
 * return 1 on success, 0 if track wasn't added
 */

int db_add_dir (sqlite3 *db, const char *path); // load a directory into the database
int db_add_file (sqlite3 *db, const char *file); // load a file into the database

struct entry *db_load_entry_from_file (const char *file); // create struct entry from the given file

void db_close (sqlite3 *db); // close the database and free memory

#endif
