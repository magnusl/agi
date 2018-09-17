#include <agi/interpreter.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <SDL.h>
#include <assert.h>

#define WINDOW_WIDTH (320 * 4)
#define WINDOW_HEIGHT (200 * 2)

static uint32_t ColorTable[] = {
    0xFF000000, // black
    0xFFAA0000, // blue
    0xFF00AA00, // green
    0xFFAAAA00, // cyan
    0xFF0000AA, // red
    0xFFAA00AA, // magenta
    0xFF00AA55, // brown
    0xFFAAAAAA, // light gray
    0xFF555555, // dark grey
    0xFFFF5555, // bright blue
    0xFF55FF55, // bright green
    0xFFFF55FF, // bright cyan
    0xFF5555FF, // bright red
    0xFFFF55FF, // bright magenta
    0xFF55FFFF, // bright yellow
    0xFFFFFFFF, // white  
};

void DrawPictureToSurface(
    SDL_Surface* surface,
    const uint8_t* pixels,
    size_t width,
    size_t height)
{
    SDL_LockSurface(surface);

    // for each row
    size_t offset = 0;
    for(size_t y = 0; y < height; y++) {
        // get a pointer to the beginning of the line
        uint32_t* rowp = reinterpret_cast<uint32_t*>(
            reinterpret_cast<uint8_t*>(surface->pixels) + (y * (surface->pitch)));
        // for each pixel in the row
        for(size_t x = 0; x < width; ++x) {
            *rowp++ = ColorTable[pixels[offset++] & 0x0F];
        }
    }
    SDL_UnlockSurface(surface);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        return -1;
    }

    const boost::filesystem::path path(argv[1]);
    agi::Interpreter interpreter(path);


    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "AGI",                             // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        WINDOW_WIDTH,                      // width, in pixels
        WINDOW_HEIGHT,                     // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );    

    if (!window) {
        std::cerr << "Failed to create SDL window" << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create SDL renderer" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

        // create a texture
    SDL_Texture* fbTexture = nullptr;
    SDL_Texture* priorityTexture = nullptr;

    SDL_Surface* framebuffer = SDL_CreateRGBSurface(0, 320, 200, 32, rmask, gmask, bmask, amask);
    assert(framebuffer);
    SDL_Surface* prioritySurface = SDL_CreateRGBSurface(0, 160, 200, 32, rmask, gmask, bmask, amask);
    assert(prioritySurface);

    SDL_Event e;
    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }

            if (e.type == SDL_KEYDOWN){
                interpreter.OnKeyPress(e.key.keysym);
            }
        }

        auto uar = interpreter.StartCycle();
        if (uar) {
            assert(false);
        }

        // get the framebuffer
        auto& fb = interpreter.GetFramebuffer();
        DrawPictureToSurface(framebuffer, fb.GetPictureBuffer().data(), 320, 200);
        DrawPictureToSurface(prioritySurface, fb.GetPriorityBuffer().data(), 160, 200);

        if (fbTexture) {
            SDL_DestroyTexture(fbTexture);
        }
        fbTexture = SDL_CreateTextureFromSurface(renderer, framebuffer);
        if (priorityTexture) {
            SDL_DestroyTexture(priorityTexture);

        }
        priorityTexture = SDL_CreateTextureFromSurface(renderer, prioritySurface);

        SDL_Rect fbRect;
        fbRect.x = 0;
        fbRect.y = 0;
        fbRect.w = 640;
        fbRect.h = 400;

        SDL_Rect pRect;
        pRect.x = 640;
        pRect.y = 0;
        pRect.w = 640;
        pRect.h = 400;

        SDL_RenderClear(renderer);
        if (fbTexture) {
            SDL_RenderCopy(renderer, fbTexture, nullptr, &fbRect);
        }
        if (priorityTexture) {
            SDL_RenderCopy(renderer, priorityTexture, nullptr, &pRect);
        }
        SDL_RenderPresent(renderer);

        SDL_Delay(interpreter.GetCycleDelay());
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
}