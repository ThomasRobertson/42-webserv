#include <stdio.h>

int main(int argc, char **arg)
{
    if (argc > 1)
        printf("%s\n", arg[1]);
    else
        printf("CGI SCRIPT: NO ARGS\n");
}