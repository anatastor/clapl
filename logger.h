
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#define DEBUG_MODE 0


FILE *LOGGER_FILE;
char *LOGGER_PATH;

typedef enum
{
    LOG_DMSG = 0,
    LOG_MSG = 1,
    LOG_WARNING,
    LOG_ERROR,
    LOG_ERROR_MALLOC,
    LOG_ERROR_PARAM,
    LOG_ERROR_IO
} LOG_CODE;


void logcmd (const LOG_CODE code, char *fmt, ...);
/**
 * @func
 * @param[in] code of the log message
 * @param[in] format of the message, use it like printf()
 * @param[in] ... further parameters defined by fmt
 */

#endif
