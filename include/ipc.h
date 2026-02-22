#ifndef IPC_H
#define IPC_H

#include "forward.h"

extern int wmfd;

void ipc_init();

void ipc_handle(WM *wm);

#endif 
