#!/bin/sh

#CFLAGS="-Wall -Wextra -Werror -O3 -ggdb -std=c++20 -Wno-overflow -Wno-format-security"
CFLAGS="-Wall -Wextra -Werror -O0 -ggdb -std=c++20 -Wno-overflow -Wno-format-security"

g++ $CFLAGS -o vannes vannes.cpp
