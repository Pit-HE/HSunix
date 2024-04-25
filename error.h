
#ifndef __ERROR_H__
#define __ERROR_H__



typedef enum kernelErrorCodeList
{
    errNull = 0,
/***************************/
    /* trap and interrupt */
    errTrapStatus,
    errInterruptState,
    errInterruptEnable,
    errInterruptDisbale,
    errInterruptNumNest,
    /** vm **/
    errVirtualAddress,
    errViritualAddressMap,
    errInvalidAddressSize,
    errInvalidPteState,
    /** printf **/
    errParameterFormat,
    /* other */
    errProcessState,
/***************************/
    errMax,
}errCode;


#endif
