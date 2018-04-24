
#include "clapl.h"


void load_config (configparser *cp)
{
    // loading the configuration file
    LOGGER_FILE = stdout; 
    chdir(getenv("HOME")); // goto /home/user/

    int ret = mkdir(".config/clapl", 0700); // create '.config/clapl/ if it does not exits
    if (ret != 0 && ret != -1)
        logcmd(LOG_MSG, "could not create local config directory");

    struct stat sb;
    ret = stat(".config/clapl/config", &sb); // check if config file exists
    if (ret != 0) 
    { 
        fprintf(stderr, "could not load config file\ncheck ~/.config/clapl/config");
        exit(1);
    }
    
    //initialize and load config file
    configparser_init(cp, ".config/clapl/config", 5, '=');
    configparser_load(cp);
}


int main (int argc, char **argv)
{
    srand(time(NULL));
    configparser *cp = malloc(sizeof(configparser));
    load_config(cp); // load config file
    
    //LOGGER_PATH = ".config/clapl/log.txt";
    LOGGER_PATH = configparser_get_string(cp, "log_file"); // set path for log file
    LOGGER_FILE = fopen(LOGGER_PATH, "w"); // open log file

    playback_init(); // initialize libav and libao
    sqlite3 *db = db_init();
    ui_init(); // initialize userinterface
    userinterface *ui = ui_create(db, cp); // create userinterface

    ui->c->commands = load_commands();

    audio *a = audio_create();
    ui_print_info(ui, a); // prints infos of the currently playing track
    ui_refresh(ui);
    pthread_t thread;

    char ch;
    char running = 1;
    while (1)
    {
        ch = getch();
        input(ui, ui->c, a, &thread, ch);
        if (ch == 'q')
            break;
    }
    /*
    while ((ch = getch()) != 'q') // main loop
        input(ui, ui->c, a, &thread, ch);
    */
    
    /*
    cache_entry_load_album(ui->c);
    cache_entry_load_tracks(ui->c);
    */
    /*
    input(ui, ui->c, a, &thread, 'p');
    input(ui, ui->c, a, &thread, 'q');
    char running = 1;
    char ch;
    while (running)
    {
        ch = getchar();
        getchar();
        if (ch == 'q')
            running = 0;
        input(ui, ui->c, a, &thread, ch);
    }
    */


    endwin(); // close ncurses
    a = audio_destroy(a);
    configparser_delete(cp);
    cache_close(ui->c);
    free(ui);
    db_close(db);
    
    fclose(LOGGER_FILE); // close log file
    return 0;
}

