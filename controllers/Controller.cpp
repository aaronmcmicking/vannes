#include "Controller.hpp"

Controller::Controller(){
    using namespace VNES_LOG;
    LOG(INFO, "Controller type KEYBOARD selected");

    type = KEYBOARD; // implicit = KEYBOARD

    START_BUTTON    = KEY_ENTER;
    SELECT_BUTTON   = KEY_SPACE;
    A_BUTTON        = KEY_X;
    B_BUTTON        = KEY_Z;
    UP_BUTTON       = KEY_UP;
    DOWN_BUTTON     = KEY_DOWN;
    LEFT_BUTTON     = KEY_LEFT;
    RIGHT_BUTTON    = KEY_RIGHT;
}

Controller::Controller(ControllerType controller_type){
    using namespace VNES_LOG;
    type = controller_type;
    switch(type){
        case KEYBOARD:
            LOG(INFO, "Controller type KEYBOARD selected");
            START_BUTTON    = KEY_ENTER;
            SELECT_BUTTON   = KEY_SPACE;
            A_BUTTON        = KEY_X;
            B_BUTTON        = KEY_Z;
            UP_BUTTON       = KEY_UP;
            DOWN_BUTTON     = KEY_DOWN;
            LEFT_BUTTON     = KEY_LEFT;
            RIGHT_BUTTON    = KEY_RIGHT;
            break;
        default:
            LOG(FATAL, "Unknown Controller type selected");
            exit(1);
            break;
    }
}

void Controller::write(uint16_t addr, uint8_t data){
    using namespace VNES_LOG;
    switch(addr){
        case 0x4016:
            if(strobe && !(data & 0x1)){ // strobe going 1->0
                cont1_buttons_strobed = cont1_buttons; 
            }

            strobe = data & 0x1;
            break;
        case 0x4017:
            // no effect
            LOG(WARN, "Write to JOY2 shift register (0x4017) has no effect (controller 2 not implemented)");
            break;
        default:
            LOG(ERROR, "Bad write as controller cannot handle address 0x%x\n", addr);
            break;
    }
}

uint8_t Controller::read(uint16_t addr){
    using namespace VNES_LOG;
    uint8_t data = 0;
    switch(addr){
        case 0x4016:
            if(!strobe){
                data = 0x80 | (cont1_buttons_strobed & 0x1);
                cont1_buttons_strobed >>= 1;
            }else{ // strobe not pulled to 0 yet, return button A status
                data = 0x4 | (cont1_buttons & 0x1);
            }
            break;
        case 0x4017:
            // no effect
            LOG(WARN, "Read from JOY2 shift register (0x4017) has no effect (controller 2 not implemented)");
            break;
        default:
            LOG(ERROR, "Bad read as controller cannot handle address 0x%x\n", addr);
            break;
    }
    return data;
}


void Controller::get_input(){
    using namespace VNES_LOG;
    switch(type){
        case KEYBOARD:
            // for testing
            START_PRESSED   = (bool)IsKeyDown(START_BUTTON);
            SELECT_PRESSED  = (bool)IsKeyDown(SELECT_BUTTON);
            A_PRESSED       = (bool)IsKeyDown(A_BUTTON);
            B_PRESSED       = (bool)IsKeyDown(B_BUTTON);
            UP_PRESSED      = (bool)IsKeyDown(UP_BUTTON);
            DOWN_PRESSED    = (bool)IsKeyDown(DOWN_BUTTON);
            LEFT_PRESSED    = (bool)IsKeyDown(LEFT_BUTTON);
            RIGHT_PRESSED   = (bool)IsKeyDown(RIGHT_BUTTON);

            // for actual use of an emulated program
            cont1_buttons = 0;
            cont1_buttons |= (int)IsKeyDown(A_BUTTON)       << (0);
            cont1_buttons |= (int)IsKeyDown(B_BUTTON)       << (1);
            cont1_buttons |= (int)IsKeyDown(SELECT_BUTTON)  << (2);
            cont1_buttons |= (int)IsKeyDown(START_BUTTON)   << (3);
            cont1_buttons |= (int)IsKeyDown(UP_BUTTON)      << (4);
            cont1_buttons |= (int)IsKeyDown(DOWN_BUTTON)    << (5);
            cont1_buttons |= (int)IsKeyDown(LEFT_BUTTON)    << (6);
            cont1_buttons |= (int)IsKeyDown(RIGHT_BUTTON)   << (7);
            break;
        default:
            LOG(FATAL, "Unknown Controller type selected");
            exit(1);
            break;
    }
}
