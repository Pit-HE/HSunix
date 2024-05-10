
#include "defs.h"


#define CLI_BUFF_SIZE   256

typedef struct cli_manage_info
{
    ringbuf_t   cb;
    int         init;
}cliInfo_t;
cliInfo_t cliState;

void cli_init (void)
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
    kRingbuf_putchar(&cliState.cb, ch);
    
    if ((ch == '\r') || (ch == '\n'))
    {
        console_wString("\r\n");
        kRingbuf_putchar(&cliState.cb, '\n');

        do
        {
            kRingbuf_getchar(&cliState.cb, &ch);
            console_wChar(ch);
        }
        while(ch != '\n');

        kRingbuf_clean(&cliState.cb);
        console_wString("sh: ");
    }
    /* TODO: Add the complete command line interaction */
}

