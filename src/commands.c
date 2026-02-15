#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "wm.h"
#include "commands.h"
#include "layout.h"

typedef struct{
    char *name;
    void (*func)(const Arg *);
    Arg arg;
}Command;

const char *termcmd[] = {"xterm", NULL};

void spawn(const Arg *arg){
    if(fork() == 0){
        setsid(); // Create a new session for the child
        execvp(arg->cparr[0], (char *const *)arg->cparr);

        //If child fails
        perror("execvp failed");
        _exit(1); // A safe way to terminate the forked child
    }
}

Command commands[] = {
    {"focus_right", focus_direction, {.i = DIR_RIGHT}},
    {"focus_left", focus_direction, {.i = DIR_LEFT}},
    {"focus_down", focus_direction, {.i = DIR_DOWN}},
    {"focus_up", focus_direction, {.i = DIR_UP}},
    {"kill", kill_window, {0}},
    {"spawn", spawn, {.cparr = termcmd}},
    {"set_master", set_master, {0}},
    {"rotate", rotate, {.i = LAYOUT_MASTER}},
    {"unmap", unmap, {0}},
    {"resize_right", resize, {.i = DIR_RIGHT}},
    {"resize_left", resize, {.i = DIR_LEFT}},
    {"resize_up", resize, {.i = DIR_UP}},
    {"resize_down", resize, {.i = DIR_DOWN}}
};

void dispatch_command(const char *cmd){
    for(size_t i = 0; i < (sizeof(commands) / sizeof(Command)); i++){
        if(strcmp(commands[i].name, cmd) == 0){
            commands[i].func(&commands[i].arg);
            return;
        }
    }
    fprintf(stderr, "WARN: Command not found");
}
