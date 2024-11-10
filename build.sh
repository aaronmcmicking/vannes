#!/bin/sh

RAYLIB="-lraylib -lGL -lglfw -lm -lpthread -ldl -lrt -lX11"

#CFLAGS="-Wall -Wextra -Werror -O3 -ggdb -std=c++20 -Wno-overflow -Wno-format-security"
if [ "$1" = "fast" ]; then
    echo building with high optimization
    CFLAGS="-Wall -Wextra -Werror -fsanitize=undefined -O3 -ggdb -std=c++20 -Wno-overflow -Wno-format-security"
else
    CFLAGS="-Wall -Wextra -Werror -fsanitize=undefined -O0 -ggdb -std=c++20 -Wno-overflow -Wno-format-security"
fi

g++ $CFLAGS -o vannes vannes.cpp $RAYLIB
