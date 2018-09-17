#include <agi/directory.h>
#include <agi/volume.h>
#include <agi/logic.h>
#include <agi/picture.h>
#include <iostream>
#include <map>
#include <sstream>
#include <SDL.h>

#define WINDOW_WIDTH (800)
#define WINDOW_HEIGHT (1000)

using VolumeCache = std::map<unsigned, std::shared_ptr<agi::Volume> >;

std::shared_ptr<agi::Volume> FindVolume(
    const std::string& basepath, unsigned volume, VolumeCache& cache)
{
    auto it = cache.find(volume);
    if (it != cache.end()) {
        return it->second;
    }
    else {
        std::stringstream ss;
        ss << basepath << "/VOL." << volume;
        auto result = agi::LoadVolume(ss.str());
        if (result) {
            // found the volume, store it in the cache
            cache[volume] = result;
        }
        return result;
    }
}

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

/**
 * \brief   Draws a 160x200 AGI picture to the SDL surface
 */
void DrawPictureToSurface(
    SDL_Surface* surface,
    const std::array<uint8_t, 32000>& pixels)
{
    SDL_LockSurface(surface);

#if 1
    // for each row
    size_t offset = 0;
    for(size_t y = 0; y < 200; y++) {
        // get a pointer to the beginning of the line
        uint32_t* rowp = reinterpret_cast<uint32_t*>(
            reinterpret_cast<uint8_t*>(surface->pixels) + (y * (surface->pitch)));
        // for each pixel in the row
        for(size_t x = 0; x < 160; ++x) {
            *rowp++ = ColorTable[pixels[offset++]];
        }
    }
    #endif
    SDL_UnlockSurface(surface);
}

bool RenderPicture(const std::string& basepath, size_t pictureIndex, agi::Framebuffer& framebuffer)
{
    // first load the script directory
    std::vector<agi::DirectoryEntry> directory;
    try {
        const std::string logdir = basepath + "/PICDIR";
        agi::ParseDirectoryFile(logdir, directory);
    }
    catch(std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return false;
    }

    VolumeCache cache;
    if (pictureIndex < directory.size()) {
        const auto& entry = directory.at(pictureIndex);
        if (auto volume = FindVolume(basepath, entry.volume, cache)) {
            if (entry.offset < volume->data.size()) {
                agi::DrawPictureResource(volume->data, entry.offset, framebuffer);
                return true;
            }
            else {
                return false;
            }
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << std::endl;
        return -1;
    }

    // render the specified picture to the framebuffer
    agi::Framebuffer framebuffer;
    if (!RenderPicture(argv[1], atoi(argv[2]), framebuffer)) {
        std::cerr << "Failed to render picture" << std::endl;
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "AGI Picture viewer",              // window title
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

    // Create the surfaces
    SDL_Surface* pictureSurface = nullptr;
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

    // create a surface for the actual AGI picture
    pictureSurface = SDL_CreateRGBSurface(0, 160, 200, 32, rmask, gmask, bmask, amask);
    assert(pictureSurface);

    // draw the AGI picture to the surface
    DrawPictureToSurface(pictureSurface, framebuffer.GetPriorityBuffer());

    // create a texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, pictureSurface);
    assert(texture);

    SDL_Event e;
    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN){
                quit = true;
            }
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
}
