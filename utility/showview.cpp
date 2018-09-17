#include <agi/directory.h>
#include <agi/volume.h>
#include <agi/logic.h>
#include <agi/picture.h>
#include <agi/view.h>
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

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static const uint32_t rmask = 0xff000000;
    static const uint32_t gmask = 0x00ff0000;
    static const uint32_t bmask = 0x0000ff00;
    static const uint32_t amask = 0x000000ff;
#else
    static const uint32_t rmask = 0x000000ff;
    static const uint32_t gmask = 0x0000ff00;
    static const uint32_t bmask = 0x00ff0000;
    static const uint32_t amask = 0xff000000;
#endif

SDL_Surface* DrawLoop(const agi::Loop& loop, size_t loopIndex)
{
    // calculate the width and height of all the cels
    size_t w = 0;
    size_t h = 0;
    for(auto& cel : loop.cels) {
        w += cel.width;
        h = std::max(h, static_cast<size_t>(cel.height));
    }
    // allocate the surface
    auto surface = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
    if (!surface) {
        return nullptr;
    }

    SDL_LockSurface(surface);
    // clear the surface
    SDL_memset(surface->pixels, 0, surface->h * surface->pitch);
    // now paint each cel, start with x = 0
    size_t penPosition = 0;
    for(auto& cel : loop.cels) {
        // draw the cel at (position, 0)
        for(uint8_t y = 0; y < cel.height; ++y) {
            uint32_t* ptr = reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(surface->pixels) + (y * (surface->pitch))) + penPosition;
            

            for(uint8_t x = 0; x < cel.width; ++x) {
                uint8_t pixel;
                if (cel.mirrored && (cel.mirrorLoop != loopIndex)) {
                    // draw mirrored 
                    pixel = cel.pixels[(y * cel.width) + (cel.width - x - 1)];
                }
                else {
                    // draw normal
                    pixel = cel.pixels[(y * cel.width) + x];
                }
                if (pixel != cel.colorKey) {
                    // not transparent, so draw the pixel
                    *ptr = ColorTable[pixel & 0x0f];
                }
                ++ptr; // continue with next pixel
            }
        }
        // move left to the next point
        penPosition += cel.width;
    }

    SDL_UnlockSurface(surface);
    return surface;
}

std::vector<SDL_Surface*> DrawView(const std::string& basepath, size_t viewIndex)
{
    std::vector<agi::DirectoryEntry> directory;
    try {
        const std::string logdir = basepath + "/VIEWDIR";
        agi::ParseDirectoryFile(logdir, directory);
    }
    catch(std::exception& e) {
        assert(false);
        std::cerr << "Caught exception: " << e.what() << std::endl;
        throw;
    }

    std::vector<SDL_Surface*> result;   
    VolumeCache cache;
    if (viewIndex < directory.size()) {
        const auto& entry = directory.at(viewIndex);
        if (auto volume = FindVolume(basepath, entry.volume, cache)) {
            if (entry.offset < volume->data.size()) {
                agi::Source source(volume->data.data(), volume->data.size(), entry.offset);
                
                agi::View view;
                agi::ParseViewResource(source, view);

                for(size_t i = 0; i < view.loops.size(); ++i) {
                    if (auto surface = DrawLoop(view.loops[i], i)) {
                        result.push_back(surface);
                    }
                }
            }
            else {
                assert(false);
            }
        }
        else {
            assert(false);
        }
    }

    return result;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << std::endl;
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO);

    auto viewSurfaces = DrawView(argv[1], atoi(argv[2]));

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

    
    std::vector<SDL_Texture*> textures;
    for(auto surface : viewSurfaces) {
        textures.push_back(SDL_CreateTextureFromSurface(renderer, surface));
    }

    size_t index = 0;

    SDL_Event e;
    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                if ((index + 1) < textures.size()) {
                    ++index;
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN){
                quit = true;
            }
        }
        SDL_RenderClear(renderer);

        if (textures[index]) {
            SDL_Rect target;
            target.x = 100;
            target.y = 100;
            target.w = viewSurfaces[index]->w;
            target.h = viewSurfaces[index]->h;
            SDL_RenderCopy(renderer, textures[index], nullptr, &target);
        }
        SDL_RenderPresent(renderer);
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
}
