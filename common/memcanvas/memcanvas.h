#ifndef MEMCANVAS_H
#define MEMCANVAS_H

#include <SDL2/SDL.h>
#include <stdint.h>

typedef uint32_t pix_t;

static const pix_t R_MASK = 0x00ff0000;
static const pix_t G_MASK = 0x0000ff00;
static const pix_t B_MASK = 0x000000ff;
static const pix_t O_MASK = 0xff000000;

const char Default_MemCanvas_Name[] = "MemCanv";

class MemCanvas {
public:
    int Init(pix_t* pixs_ptr, int des_width, int des_height, size_t scale = 1, const char* name = Default_MemCanvas_Name);
    int Quit();
    int Update();
private:
    bool inited = false;
    int width =  0;
    int height = 0;
    size_t scl = 0;
    pix_t*        pixs = nullptr;
    pix_t*     current = nullptr;
    SDL_Window    *win = nullptr;
    SDL_Surface   *srf = nullptr;
    SDL_Renderer *rndr = nullptr;
};

#endif // MEMCANVAS_H
