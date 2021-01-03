/* platform.h structures and functions specific to the X platform 

  Peter Howkins 21/12/2004
*/
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "SDL.h"

/* An upper limit on how big to support monitor size, used for
   allocating a scanline buffer and bounds checking. It's much
   more than a VIDC1 can handle, and should be pushing the RPC/A7000
   VIDC too, if we ever get around to supporting that. */
#define MaxVideoWidth 2048
#define MaxVideoHeight 1536

/* The size of the border surrounding the video data. */
#define VIDC_BORDER 10

struct plat_display {
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  void* pixels;
  int pitch;
  int width;
  int height;
};

extern struct plat_display PD;

// extern void Resize_Window(ARMul_State *state,int x,int y);

// extern unsigned int vidc_col_to_x_col(unsigned int col);

// extern void hostdisplay_change_focus(int focus);

// extern void UpdateCursorPos(ARMul_State *state,int xscale,int xoffset,int yscale,int yoffset);

// extern const DisplayDev true_DisplayDev;
// extern const DisplayDev pseudo_DisplayDev;

#endif /* _PLATFORM_H_ */
