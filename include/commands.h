#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum{
    CMD_OK,
    CMD_BADARG
}CmdResult;

CmdResult dispatch_command(const char *cmd);

#endif
