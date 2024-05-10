
#ifndef __ERROR_H__
#define __ERROR_H__



typedef enum kernel_error_service_list
{
    eSVC_Null = 0,

    eSVC_Trap,
    eSVC_Interrupt,
    eSVC_VirtualMem,
    eSVC_Printf,
    eSVC_Ringbuf,
    eSVC_Process,
    eSVC_Timer,
    eSVC_Syscall,
/**********/
    eSVC_Max,
}eService;


typedef enum kernel_error_code_list
{
    E_NULL = 0,
    E_STATUS,
    E_INTERRUPT,
    E_PARAM,
    E_PROCESS,
    E_UNSPECIFIED,
    E_BAD_PROC,
    E_INVAL,
    E_KILLED,
    E_TIMEOUT,
    E_BUSY,
    E_DANGER,
/**********/
    E_MAX,
}eCode;



#endif
