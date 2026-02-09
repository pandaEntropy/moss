#include "wm.h"
#include "commands.h"

#include <string.h>

typedef struct{
    char *name;
    void (*func)(const Arg *);
    Arg arg;
}Command;

static const char *termcmd[] = {"xterm", NULL};

Command commands[] = {
    {"focus_right", focus_direction, {.i = DIR_RIGHT}},
    {"focus_left", focus_direction, {.i = DIR_LEFT}},
    {"focus_down", focus_direction, {.i = DIR_DOWN}},
    {"focus_up", focus_direction, {.i = DIR_UP}},
    {"kill", kill_window, {0}},
    {"spawn", spawn, {.cparr = termcmd}},
    {"set_master", set_master, {0}}
};

CmdResult dispatch_command(const char *cmd){
    for(int i = 0; i < (sizeof(commands) / sizeof(Command)); i++){
        if(strcmp(commands[i].name, cmd) == 0){
            commands[i].func(&commands[i].arg);
            return CMD_OK;
        }
    }
    return CMD_BADARG;
}
