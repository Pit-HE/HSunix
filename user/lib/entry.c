
#include "libc.h"


void _main (void)
{
    extern int main (void);
    exit(main());
}
