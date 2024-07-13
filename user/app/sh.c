
#include "libc.h"
#include "shell.h"

int main (int argc, char *argv[])
{
    init_shell();

    while(1)
    {
        shell_main();
    }
    return 0;
}
