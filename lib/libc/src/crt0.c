/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <stddef.h>
#include <stdlib.h>

extern int main(int argc, const char** argv);

int _start()
{
    int st = main(0, NULL);

    exit(st);
}
