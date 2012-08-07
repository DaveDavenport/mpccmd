#ifndef __MPCCMD_H__
#define __MPCCMD_H__
#include <mpd/client.h>


extern struct mpd_connection *connection;
typedef struct _State State;
typedef struct _CommandModule CommandModule;


struct _State{
    const CommandModule   *cmd;
    const char      *command;
    char            *arguments;
};

#include "mpc_player.h"

void handle_connect(State *st);
void handle_quit(State *st);
#endif
