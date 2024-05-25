
#include "defs.h"


#define CLI_BUFF_SIZE   256

typedef struct cli_manage_info
{
    ringbuf_t   cb;
    int         init;
}cliInfo_t;
cliInfo_t cliState;

void init_cli (void)
{
    char *buf;

    buf = (char *)kalloc(CLI_BUFF_SIZE);
    if (buf != NULL)
    {
        cliState.init = 1;
        kRingbuf_init(&cliState.cb, buf, CLI_BUFF_SIZE);
    }
}

void cli_main (void)
{
    char ch;

    if (cliState.init == 0)
        return;

    ch = console_rChar();
    if (ch == '\r')
        ch = '\n';
    kRingbuf_putChar(&cliState.cb, ch);

    if (ch == '\n')
    {
        kprintf("\r\n");

        do
        {
            kRingbuf_getChar(&cliState.cb, &ch);
            console_wChar(ch);
        }
        while(ch != '\n');

        kprintf("sh: ");
    }
    /* TODO: Add the complete command line interaction */
}

