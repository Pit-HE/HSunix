
#ifndef __MSG_H__
#define __MSG_H__


#include "types.h"


void init_msg (void);
int k_msgget (int key, int msgflg);
int k_msgsnd (int msqid, void *msgp, uint msgsz, int msgflg);
int k_msgrcv (int msqid, void *msgp, uint msgsz, int msgtyp, int msgflg);
int k_msgctl (int msqid, int cmd, void *uptr);


#endif
