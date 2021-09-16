#include <cassert>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "fps_counter.h"
#include "map.h"
#include "player.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

class GameState {
    SDL_Renderer * const renderer;
    FPS_Counter fps_counter;
    Map map;
    Player player;
    TimeStamp timestamp;
public:
    GameState(SDL_Renderer *r) : renderer(r), fps_counter(r), map(r), player(r), timestamp(Clock::now()) {}
    void update(double fpsRegulator = -1.);
};

void GameState::update(const double fpsRegulator) {
    assert(renderer != nullptr);

    player.handle_keyboard(); // no need for the event variable, direct keyboard state polling

    const auto now = Clock::now();
    const double dt = std::chrono::duration<double>(now - timestamp).count();

    if (fpsRegulator > 0.0) {
        // FPS regulation (desktop mode only)
        if (const auto targetms = 1.0 / fpsRegulator; dt < targetms) {
            std::this_thread::sleep_for(std::chrono::microseconds(long((targetms - dt)*1e3)));
            return;
        }
    }

    timestamp = now;

    player.update_state(dt, map); // gravity, movements, collision detection etc

    SDL_RenderClear(renderer); // re-draw the window
    fps_counter.draw();
    player.draw();
    map.draw();
    SDL_RenderPresent(renderer);
}

#ifndef __EMSCRIPTEN__
void main_loop(GameState &s) {
    while (1) { // main game loop
        SDL_Event event; // handle window closing
        if (SDL_PollEvent(&event) && (SDL_QUIT==event.type || (SDL_KEYDOWN==event.type && SDLK_ESCAPE==event.key.keysym.sym)))
            break; // quit

        s.update(60 /* target fps */);
    }
}
#endif

int main() {
    SDL_SetMainReady(); // tell SDL that we handle main() function ourselves, comes with the SDL_MAIN_HANDLED macro
    if (SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;
    if (SDL_CreateWindowAndRenderer(1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS, &window, &renderer)) {
        std::cerr << "Failed to create window and renderer: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_SetWindowTitle(window, "SDL2 game blank");
    SDL_SetRenderDrawColor(renderer, 210, 255, 179, 255);

    {
        // all interesting things happen here
        GameState s(renderer);

#ifdef __EMSCRIPTEN__
        try {
            emscripten_set_main_loop_arg([](void *arg){
                static_cast<GameState *>(arg)->update();
            }, &s, -1 /* auto-fps */, true);
        } catch (...) {}
#else
        main_loop(s);
#endif
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
