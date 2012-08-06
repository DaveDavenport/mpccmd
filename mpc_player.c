#include <stdio.h>
#include <assert.h>
#include "mpccmd.h"

void handle_next(State *st)
{
    assert(connection != NULL);
    printf("Skipping song\n");
    mpd_run_next(connection);
}
void handle_previous(State *st)
{
    assert(connection != NULL);
    printf("Skipping song\n");
    mpd_run_next(connection);
}

void handle_pause(State *st)
{
    assert(connection != NULL);
    printf("Toggling paused state\n");
    mpd_run_toggle_pause(connection);

}
void handle_stop(State *st)
{
    assert(connection != NULL);
    printf("Stop playback\n");
    mpd_run_stop(connection);

}

void handle_play(State *st)
{
    assert(connection != NULL);
    printf("Play song\n");
    if(st->arguments == NULL || st->arguments[0] == '\0')
    {
        mpd_run_play(connection);
    }
}


