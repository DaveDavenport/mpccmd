#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <mpd/client.h>
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


void handle_add(State *st)
{
    assert(connection != NULL);

    if(st->arguments != NULL && strlen(st->arguments) > 0)
    {
        int length = strlen(st->arguments);
        char *start = st->arguments;
        char *token = index(st->arguments, (int)'=');
        if(token != NULL)
        {
            mpd_search_add_db_songs(connection, false);
            do {
                char *iter;
                *token = '\0';
                for ( iter = token; iter>= start; iter--) {
                    if(*iter == ' ') break;
                }
                char *a = strndup(iter+1, token-iter);


                start = token+1;
                token = index(start, '=');
                if(token == NULL) {
                    iter = index(start, '\0');
                }else{
                    for ( iter = token; iter>= start; iter--) {
                        if(*iter == ' ') break;
                    }
                }

                char *b = strndup(start, iter-start);
                iter = strtok(b, " ");
                while(iter != NULL) {
                     mpd_search_add_tag_constraint(connection,MPD_OPERATOR_DEFAULT, mpd_tag_name_iparse(a), iter);
                    iter = strtok(NULL, " ");
                }

                printf("%li '%s'--'%s'\n", iter-start, a,b);
                free(a);
                free(b);
            }while(token != NULL);
            printf("aap\n");
            mpd_search_commit(connection);
            struct mpd_song *song = NULL;
            while((song = mpd_recv_song(connection)) != NULL) mpd_song_free(song);
            printf("noot\n");
        }else{

        }
    }
}
