#!/bin/sh

#RAYLIB="-lraylib -lGL -lglfw -lm -lpthread -ldl -lrt -lX11"
RAYLIB="-lraylib -lGL -lm -lpthread -ldl -lrt -lX11"

if [ "$1" = "fast" ]; then
    echo building with high optimization
    CFLAGS="-Wall -Wextra -Werror -fsanitize=undefined -O2 -ggdb -std=c++20 -Wno-overflow -Wno-format-security"
else
    CFLAGS="-Wall -Wextra -Werror -fsanitize=undefined -O0 -ggdb -std=c++20 -Wno-overflow -Wno-format-security"
fi

g++ $CFLAGS -o vannes vannes.cpp $RAYLIB
