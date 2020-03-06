#include <stdio.h>

#include "exename.h"

int main(void)
{
    char *s;

    s = exename();
    printf("%s\n", s ? s : "N/A");
    return 0;
}
