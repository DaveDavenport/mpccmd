#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <mpd/async.h>
#include "mpccmd.h"


struct mpd_connection *connection = NULL;
bool quit = false;

typedef enum _StateFlags {
    NONE = 0,
    NEEDS_CONNECTION = 1

}StateFlags;

struct _CommandModule {
    const char *name;
    void (*handler)(State *st);
    StateFlags      flags;
};

static const CommandModule mpccmd_commands[]  = {
    {"connect",     &handle_connect,    NONE },
    {"disconnect",  NULL,               NEEDS_CONNECTION},
    {"play",        &handle_play,       NEEDS_CONNECTION},
    {"pause",       &handle_pause,      NEEDS_CONNECTION},
    {"next",        &handle_next,       NEEDS_CONNECTION},
    {"previous",    &handle_previous,   NEEDS_CONNECTION},
    {"stop",        &handle_stop,       NEEDS_CONNECTION},
    {"quit",        &handle_quit,       NONE},
    {"help",        NULL,               NONE},
    {NULL,        NULL}
};


bool state_is_valid(const State *st)
{
    return (st->cmd != NULL);
}

State *state_new()
{
    State *st = (State *)malloc(sizeof(*st));
    st->cmd = NULL;
    st->command = NULL;
    st->arguments = NULL;
    return st;
}

void state_free(State ** state)
{
    if(state == NULL || *state == NULL) return;
    free(*state);
    *state = NULL;
}


void handle_quit(State *st)
{
    quit = true;
}
/**
 * Handle connect command.
 */
void handle_connect(State *st)
{
    printf("Handle connect:  '%s'\n", st->arguments);
    if(connection != NULL) {
        printf("Allready connected\n");
    }

    if(st->arguments == NULL || st->arguments[0] == '\0')
    {
        connection = mpd_connection_new(NULL, 6600, 1e3);
    }else{
        connection = mpd_connection_new(st->arguments, 6600, 1e3);
    }

    if(mpd_connection_get_error(connection) != MPD_ERROR_SUCCESS)
    {
        printf("Failed to connect: %i\n", mpd_connection_get_error(connection));
        mpd_connection_free(connection);
        connection =  NULL;
    }
}

void state_execute(State *st)
{
    if(st == NULL || st->cmd == NULL) return;
    if(st->cmd->handler != NULL)
    {
        if(st->cmd->flags&NEEDS_CONNECTION) {
            // Check connection.
            if(connection == NULL) {
                printf("This commands requires you to be connected to a MPD server.\n");
                return;
            }
        }
        st->cmd->handler(st);
    }
}


static State *parse_state(const char *text)
{
    State *state = state_new();
    const char *index = text;

    for ( int i = 0; mpccmd_commands[i].name != NULL ; i++) {
        int cmd_length = strlen(mpccmd_commands[i].name);
        if(strncasecmp(mpccmd_commands[i].name, text, cmd_length) == 0) {
            if(text[cmd_length] == '\0'  || whitespace(text[cmd_length]))
            {
                state->cmd = &(mpccmd_commands[i]);
                state->command = mpccmd_commands[i].name;

                const char *p = index+strlen(mpccmd_commands[i].name);
                // Skip leading whitespace.
                while(whitespace(*p)) p++;
                state->arguments = strdup(p);
                break;
            }
        }
    }

    return state;
}


/**
 * @brief
 *
 * @param text
 * @param state
 *
 * @return
 */
static char* my_generator(const char* text, int state)
{
    static int list_index, len;
    const char *name;
    if (!state) {
        list_index = 0;
        len = strlen (text);
    }

    while ((name = mpccmd_commands[list_index].name) != NULL ) {
        list_index++;

        if (strncasecmp (name, text, len) == 0)
            return (strdup(name));
    }

    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}

/**
 * @brief
 *
 * @param text
 * @param start
 * @param end
 *
 * @return
 */
static char** my_completion( const char * text , int start,  int end)
{
    char **matches;

    matches = (char **)NULL;
    State *st = parse_state(rl_line_buffer);

    if(!state_is_valid(st))
        matches = rl_completion_matches ((char*)text, &my_generator);

    state_free(&st);
    return (matches);

}
int custom_complete(int a, int b)
{
    State *st = parse_state(rl_line_buffer);
    int val = 0;
    if(!state_is_valid(st))
    {
        val = rl_complete(a,b);
    }else {
        val = rl_insert(a,b);
    }
    return val;
}


static bool has_idle = false;
void my_rlhandler(char* line)
{
    if(line==NULL)
    {
        // Ctrl-D will allow us to exit nicely
        printf("\nNULLBURGER\n");
    }
    else
    {
        if(connection != NULL) {
            if(has_idle) {
                mpd_run_noidle(connection);
                has_idle = false;
            }
        }
        State *st = parse_state(line);
        state_execute(st);
        state_free(&st);
        if(*line!=0){
            // If line wasn't empty, store it so that uparrow retrieves it
            add_history(line);
        }
    }
}

/**
 * @brief
 */
void run()
{
    char shell_prompt[1024], *input;

    // Set custom completion.
    rl_attempted_completion_function = my_completion;
    rl_menu_completion_entry_function= my_generator;
    rl_completion_entry_function = NULL;
    rl_attempted_completion_over=1;

    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);
    rl_bind_key(' ', custom_complete);

    snprintf(shell_prompt, 1024, "MPC: ");
    // Install the handler
    rl_callback_handler_install(shell_prompt, (rl_vcpfunc_t*) &my_rlhandler);
    // While loop.
    fd_set rfds;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */


    while (!quit ) {
        int length = 1;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        if(connection != NULL)
        {
            // Start idle mode.
            if(!has_idle) {
                mpd_send_idle(connection);
                has_idle = true;
            }
            // wait on fd for input.
            FD_SET(mpd_connection_get_fd(connection), &rfds);
            length = (length > (mpd_connection_get_fd(connection)+1))?length:(mpd_connection_get_fd(connection)+1);
        }

        retval = select(length, &rfds, NULL, NULL, NULL);
        if(FD_ISSET(0, &rfds)) {
            rl_callback_read_char();
        }
        else if(connection != NULL && length > 1 &&  FD_ISSET(mpd_connection_get_fd(connection), &rfds)){
            int mask = mpd_recv_idle(connection, true);
            if(mpd_connection_get_error(connection)){
                printf("%s\n", mpd_connection_get_error_message(connection));
                exit(0);
            }

            struct mpd_song *sg = mpd_run_current_song(connection);
            if(sg) {
                printf("\nCurrent Song: %s - %s\n", mpd_song_get_tag(sg,MPD_TAG_ARTIST, 0),  mpd_song_get_tag(sg,MPD_TAG_TITLE, 0));
                rl_reset_line_state();
                rl_on_new_line();
                rl_redisplay();
            }
            has_idle = false;
        }
        else {
            printf("\nTick:\n");
            rl_reset_line_state();
            rl_on_new_line();
            rl_redisplay();
        }
    }
    rl_callback_handler_remove();
}



int main()
{
    // Cleanup
    run();
}
