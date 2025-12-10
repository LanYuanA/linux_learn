#include "stdlib.h"
#include "stdio.h"
#include "signal.h"

void signal_handler(int num)
{
    printf("\r\nSIGINT signal!\r\n");
    exit(0);
}
int main()
{
    signal(SIGINT,signal_handler);
    while(1);
    return 0;
}