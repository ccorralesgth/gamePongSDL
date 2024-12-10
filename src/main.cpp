#include <iostream>
#include <SDL2/SDL.h>

//option#1 (without main parameters) SDL redefines main to SDL_main
// // #ifdef main
// // #undef main
// // #endif

// // int main() {
// //     std::cout << "Hello, World!" << std::endl;
// //     return 0;
// // }

int main(int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "SDL initialized successfully" << std::endl;
    SDL_Quit();
    return 0;
}