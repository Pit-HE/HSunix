
#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum kernel_error_service_list
{
    errServiceNull = 0,
/************************************/
    errSVC_Trap,
    errSVC_Interrupt,
    errSVC_VirtualMem,
    errSVC_Printf,
    errSVC_Ringbuf,
    errSVC_Process,
/************************************/
    errServiceMax,
}errService;

typedef enum kernel_error_code_list
{
    errNull = 0,
/***************************/
    errCode_Status,
    errCode_InterruptState,
    errCode_InterruptNest,
    errCode_ParamFormat,
    errCode_ParamInfo,
    errCode_ParamNullPoint,
    errCode_ProcessState,
/***************************/
    errMax,
}errCode;


#endif
