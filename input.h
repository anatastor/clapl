
#ifndef _INPUT_H_
#define _INPUT_H_

#include <unistd.h>
#include <pthread.h>

#include "cache.h"
#include "db.h"
#include "enums.h"
#include "logger.h"
#include "playback.h"
#include "ui.h"


void *playbackThread (void *vargp);
void start_playback (audio *a, cache *c, userinterface *ui, pthread_t *thread);
void input (userinterface *ui, cache *c, audio *a, pthread_t *thread, const char ch);

#endif
