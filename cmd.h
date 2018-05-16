
#ifndef _CMD_H_
#define _CMD_H_

#include "commandparser.h"
#include "ui.h"
#include "cache.h"
#include "playback.h"


cmd_table *load_commands (void);

/*
 * functions return either 1 or 0
 * the return determines if a redraw of the whole userinterface is neccessary
 */

int cmd_add (cmd_arg *args);
int cmd_seek (cmd_arg *args);
int cmd_rm (cmd_arg *args);
int cmd_rmp (cmd_arg *args);
int cmd_load (cmd_arg *args);
int cmd_sorting (cmd_arg *args);






#endif
