#include <iostream>

#include "core/RAM.cpp"
#include "core/CPU.cpp"
#include "common/log.hpp"
#include "common/util.hpp"
#include "common/nes_assert.hpp"
#include "cartridge/cartridge.cpp"
#include "mappers/Mapper000.cpp"
#include "controllers/Controller.cpp"

#include <chrono>
#include <ostream>
#include <algorithm>

#include <raylib.h>

void parse_args(int argc, char** argv, std::string& rom_filename){
    for(int i = 1; i < argc; i++){
        std::string arg {argv[i]};
        int split_pos = arg.find("=");
        std::string variable {arg.substr(0, split_pos)};
        std::string value {arg.substr(split_pos+1)}; 
        //std::cout << "found: " << variable << " = " << value << std::endl;

        if(variable == "rom"){
            rom_filename = value;
        }else if(variable == "log_level"){
            VNES_LOG::log_level = (VNES_LOG::Severity)std::atoi(value.c_str());
        }else if(variable == "log_to_file"){
            VNES_LOG::file_out = (value == "1");
            //if(value == "1"){ VNES_LOG::file_out = true; }
        }else{
            VNES_LOG::LOG(VNES_LOG::FATAL, "Unknown argument '%s'", variable.c_str());
            VNES_ASSERT(0 && "Bad argument");
        }
    }
    //std::cout << "set: " << rom_filename << std::endl;
    //std::cout << "set: " << log_level << std::endl;
}

int col2uint(Color col){
    return col.a << 24 | col.b << 16 | col.g << 8 | col.r;
}

int main(int argc, char** argv){
    using namespace VNES_LOG;

    const int NES_WIDTH = 256;
    const int NES_HEIGHT = 224;
    const int WIN_DEFAULT_WIDTH = 1024;
    const int WIN_DEFAULT_HEIGHT = 896;
    const int TARGET_FPS = 60;

    (void)NES_WIDTH;
    (void)NES_HEIGHT;
    (void)WIN_DEFAULT_WIDTH;
    (void)WIN_DEFAULT_HEIGHT;
    (void)TARGET_FPS;


    //std::string rom_filename {"roms/Super Mario Bros. (Japan, USA).nes"};
    std::string rom_filename {"roms/nestest.nes"};
    //std::string rom_filename {""};
    parse_args(argc, argv, rom_filename);
    init_log();

    Controller controller = Controller(KEYBOARD);
    Cartridge cart = Cartridge(rom_filename);
    RAM ram = RAM(cart, controller);
    ram.write(RAM::RESET_VEC, 0x00);
    ram.write(RAM::RESET_VEC + 1, 0xc0);
    //ram.write(PPU::PPU_STATUS, 0xFF); // programs wait for PPU at reset
    PPU ppu = PPU(ram, cart);
    CPU cpu = CPU(ram, ppu);

    // for nestest.nes
    ram.write(0x0002, 0);
    ram.write(0x0003, 0);

    // TEST
    //ram.write(0x0180, 0x33);

    //FILE* file = fopen("./nestest.vannes.log", "w");

    int i = 0;
    (void)i;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    //for(i = 0; i < steps_to_do; i++){
    //    cpu.step();
    //    //fprintf(file, "%4x  A:%2x X:%2x Y:%2x SP:%2x\n", cpu.program_counter, cpu.accumulator, cpu.index_X, cpu.index_Y, cpu.stack_pointer);
    //    //printf("%4x  A:%2x X:%2x Y:%2x SP:%2x\n", cpu.program_counter, cpu.accumulator, cpu.index_X, cpu.index_Y, cpu.stack_pointer);
    //    if(ram.read(0x0002)){
    //        //LOG(ERROR, "Found failing test code at 0x0002, stopping");
    //        //steps_to_do = i;
    //        //break;
    //    }
    //}
    
    uint64_t frame_cycles_to_do = 30000;
    int steps_done = 0;

    SetTraceLogLevel(LOG_ERROR);
    InitWindow(WIN_DEFAULT_WIDTH, WIN_DEFAULT_HEIGHT, "vannes");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(120);
    Texture2D text = LoadTexture("default_texture.png");
    assert(text.width == NES_WIDTH);
    assert(text.height == NES_HEIGHT);
    int pixels[NES_WIDTH*NES_HEIGHT] = {};
    (void)pixels;
    //std::fill_n(pixels, NES_WIDTH*NES_HEIGHT, col2uint(RED));
    for(int i = 0; i < NES_WIDTH*NES_HEIGHT; i++){
        pixels[i] = 0xFFFFFFFF - i;
    }

    unsigned int background_colour = col2uint((Color){30, 30, 30, 255});
    while(!WindowShouldClose()){

        // Update section
        controller.get_input();
        while(cpu.frame_cycles < frame_cycles_to_do){
            //fprintf(file, "%4x  A:%2x X:%2x Y:%2x P:%2x SP:%2x\n", cpu.program_counter, cpu.accumulator, cpu.index_X, cpu.index_Y, cpu.status_as_int(), cpu.stack_pointer);
            cpu.step();
            steps_done++;

            std::cin.get();
        }

        char pressed_keys_text[10] = "XXXXXXXX";
        pressed_keys_text[9] = '\0';
        if(controller.START_PRESSED){
            pressed_keys_text[6] = 'S';
            background_colour = col2uint(MAGENTA);
        }
        if(controller.SELECT_PRESSED){
            pressed_keys_text[7] = 's';
            background_colour = col2uint(GREEN);
        }
        if(controller.A_PRESSED){
            pressed_keys_text[1] = 'A';
            background_colour = col2uint(VIOLET);
        }
        if(controller.B_PRESSED){
            pressed_keys_text[0] = 'B';
            background_colour = col2uint(ORANGE);
        }
        if(controller.UP_PRESSED){
            pressed_keys_text[2] = 'U';
            background_colour = col2uint(RED);
        }
        if(controller.DOWN_PRESSED){
            pressed_keys_text[3] = 'D';
            background_colour = col2uint(BLUE);
        }
        if(controller.LEFT_PRESSED){
            pressed_keys_text[4] = 'L';
            background_colour = col2uint(GREEN);
        }
        if(controller.RIGHT_PRESSED){
            pressed_keys_text[5] = 'R';
            background_colour = col2uint(PURPLE);
        }

        for(int i = 0; i < 256*224; i++){
            ppu.buffer[i] = background_colour;
        }

        // Render section
        BeginDrawing();
        ClearBackground(BLACK);

        float texture_scale = std::min((float)GetScreenWidth() / NES_WIDTH, (float)GetScreenHeight() / NES_HEIGHT);
        Vector2 texture_pos = {(GetScreenWidth() - NES_WIDTH*texture_scale) / 2, 0};
        //UpdateTexture(text, pixels);
        UpdateTexture(text, ppu.buffer);

        DrawTextureEx(text, texture_pos, 0, texture_scale, WHITE);

        char res_text[100] = {};
        char texture_res_text[100] = {};
        char ratio_text[100] = {};
        char target_fps_text[100] = {};
        snprintf(res_text, 99, "resolution: (%d, %d)", GetScreenWidth(), GetScreenHeight());
        DrawText(res_text, 10, 30, 30, WHITE);
        snprintf(texture_res_text, 99, "square: (%g, %g)", NES_WIDTH*texture_scale, NES_HEIGHT*texture_scale);
        DrawText(texture_res_text, 10, 80, 30, WHITE);
        snprintf(ratio_text, 99, "square ratio w/h: %g", (NES_WIDTH*texture_scale)/(NES_HEIGHT*texture_scale));
        DrawText(ratio_text, 10, 130, 30, WHITE);
        snprintf(target_fps_text, 99, "target_fps: %d", TARGET_FPS);
        DrawText(target_fps_text, 10, 180, 30, WHITE);

        DrawText(pressed_keys_text, 10, 230, 30, WHITE);

        DrawFPS(10, 10);
        EndDrawing();

    }
    CloseWindow();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto secs = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();
    auto micro = std::chrono::duration_cast<std::chrono::microseconds> (end - begin).count();
    std::cout << frame_cycles_to_do << " frame cycles and " << steps_done << " steps took " << secs << "s = " << milli << "ms = " << micro << "us" << std::endl;
    //std::cout << frame_cycles_to_do << " frame cycles and " << steps_done << " steps took " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "ms" << std::endl;
    //std::cout << frame_cycles_to_do << " frame cycles and " << steps_done << " steps took " << std::chrono::duration_cast<std::chrono::microseconds> (end - begin).count() << "µs" << std::endl;
    printf("(cpu did %ld cycles since reset)\n", cpu.cycles_since_reset);

    uint8_t first_error_code = ram.read(0x0002);
    uint8_t second_error_code = ram.read(0x0003);
    printf("Read from 0x0002: %x\n", first_error_code);
    printf("Read from 0x0003: %x\n", second_error_code);
    printf("Note: if 0x0003 was greater than or equal to 0x004E then the test failed on an invalid opcode.\n");

    //fclose(file);

    //for(int i = 0; i < 10; i++) { ram.write(i, i); ram.write(i+0x8000, i); }
    //ram.dump();
    //cart.dump_rom();
    //LOG(DEBUG, "\nPPU cycles_since_reset: %lld\nPPU frame_cycles: %d\nPPU scanline_cycles: %d\nPPU scanlines: %d", ppu.cycles_since_reset, ppu.frame_cycles, ppu.scanline_cycles, ppu.scanlines);

    //cpu.reset();
    //LOG(DEBUG, "\nPPU cycles_since_reset: %lld\nPPU frame_cycles: %d\nPPU scanline_cycles: %d\nPPU scanlines: %d", ppu.cycles_since_reset, ppu.frame_cycles, ppu.scanline_cycles, ppu.scanlines);

    return 0;
}

