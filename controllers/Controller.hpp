#pragma once

#include <raylib.h>
#include "../common/log.hpp"

typedef enum ControllerType{
    KEYBOARD = 0,
    /* add more */
}ControllerType;

class Controller{
    public:
        bool A_PRESSED, B_PRESSED, UP_PRESSED, DOWN_PRESSED, LEFT_PRESSED, RIGHT_PRESSED, SELECT_PRESSED, START_PRESSED;

    private:
        int A_BUTTON, B_BUTTON, UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, START_BUTTON, SELECT_BUTTON;
        ControllerType type;

        bool strobe;
        uint8_t cont1_buttons;            // continually updated with new inputs
        uint8_t cont1_buttons_strobed;    // set when strobe 1->0, then right shifted on reads

    public:
        Controller();
        Controller(ControllerType controller_type);

        void write(uint16_t addr, uint8_t data);
        uint8_t read(uint16_t addr);

        void get_input();
};
