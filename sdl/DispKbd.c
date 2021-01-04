/* (c) David Alan Gilbert 1995-1999 - see Readme file for copying info
   Hacked about with for new display driver interface by Jeffrey Lee, 2011 */
/* Display and keyboard interface for the Arc emulator */

/*#define DEBUG_VIDCREGS */
// #define DEBUG_KBD
// #define DEBUG_MOUSEMOVEMENT

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "armdefs.h"
#include "arch/armarc.h"
#include "arch/keyboard.h"
#include "arch/archio.h"
#include "arch/hdc63463.h"
#ifdef SOUND_SUPPORT
#include "arch/sound.h"
#endif
#include "arch/displaydev.h"

#include "ControlPane.h"

#include "SDL.h"
#include "platform.h"

extern const DisplayDev sdl_DisplayDev;

struct plat_display PD;

const int initialWindowWidth = 640;
const int initialWindowHeight = 480;

/* XPM */
static const char *arrow[] = {
  /* width height num_colors chars_per_pixel */
  "    32    32        3            1",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "X                               ",
  "XX                              ",
  "X.X                             ",
  "X..X                            ",
  "X...X                           ",
  "X....X                          ",
  "X.....X                         ",
  "X......X                        ",
  "X.......X                       ",
  "X........X                      ",
  "X.....XXXXX                     ",
  "X..X..X                         ",
  "X.X X..X                        ",
  "XX  X..X                        ",
  "X    X..X                       ",
  "     X..X                       ",
  "      X..X                      ",
  "      X..X                      ",
  "       XX                       ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "0,0"
};

static SDL_Cursor *init_system_cursor(const char *image[])
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  int hot_x, hot_y;

  i = -1;
  for (row=0; row<32; ++row) {
    for (col=0; col<32; ++col) {
      if (col % 8) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

static void ProcessButton(ARMul_State *state)
{
	int UpNDown = sdlMouseInfo.UpNDown;
	
	
	int ButtonNum;
	
	switch (sdlMouseInfo.buttonIndex)
	{
	case 1:
		ButtonNum = 0;
		break;
		
	case 3:
		ButtonNum = 1;
	
	default:
		break;
	}
	
	if (sdlMouseInfo.buttonIndex )

	// Hey if you've got a 4 or more buttoned mouse hard luck!
	if (ButtonNum < 0)
	{
		return;
	}

	if (KBD.BuffOcc >= KBDBUFFLEN)
	{
#ifdef DEBUG_KBD
		fprintf(stderr, "KBD: Missed mouse event - buffer full\n");
#endif
		return;
	}

	/* Now add it to the buffer */
	KBD.Buffer[KBD.BuffOcc].KeyColToSend = ButtonNum;
	KBD.Buffer[KBD.BuffOcc].KeyRowToSend = 7;
	KBD.Buffer[KBD.BuffOcc].KeyUpNDown = UpNDown;
#ifdef DEBUG_KBD
	fprintf(stderr, "ProcessButton: Got Col,Row=%d,%d UpNDown=%d BuffOcc=%d\n",
			KBD.Buffer[KBD.BuffOcc].KeyColToSend,
			KBD.Buffer[KBD.BuffOcc].KeyRowToSend,
			KBD.Buffer[KBD.BuffOcc].KeyUpNDown,
			KBD.BuffOcc);
#endif
	KBD.BuffOcc++;

	sdlMouseInfo.buttonChanged = false;
}

static void MouseMoved(ARMul_State *state)
{
	int xdiff, ydiff;

	// sdlMouseInfo is deined in sdl/platform.h and set in Host_PollDisplay in sdlDisplayDriver.
	xdiff = sdlMouseInfo.xDiff;
	ydiff = -sdlMouseInfo.yDiff;

#ifdef DEBUG_MOUSEMOVEMENT
	fprintf(stderr, "MouseMoved: xdiff = %d  ydiff = %d\n",
			xdiff, ydiff);
#endif

	if (xdiff > 63)
		xdiff = 63;
	if (xdiff < -63)
		xdiff = -63;

	if (ydiff > 63)
		ydiff = 63;
	if (ydiff < -63)
		ydiff = -63;

	KBD.MouseXCount = xdiff & 127;
	KBD.MouseYCount = ydiff & 127;

	sdlMouseInfo.mouseMoved = false;

//   mouseMF = 0;
#ifdef DEBUG_MOUSEMOVEMENT
	fprintf(stderr, "MouseMoved: generated counts %d,%d xdiff=%d ydifff=%d\n", KBD.MouseXCount, KBD.MouseYCount, xdiff, ydiff);
#endif
}; /* MouseMoved */

int DisplayDev_Init(ARMul_State *state)
{
	SDL_CreateWindowAndRenderer(initialWindowWidth, initialWindowHeight, SDL_WINDOW_RESIZABLE, &PD.window, &PD.renderer);
	SDL_SetWindowTitle(&PD.window, "ArcEm");
	ChangeRenderResolution(initialWindowWidth, initialWindowHeight);
	SDL_GL_SetSwapInterval(1);
	// PD.cursor = init_system_cursor(arrow);
	// SDL_SetCursor(PD.cursor);
	// SDL_ShowCursor(SDL_ENABLE);

	return DisplayDev_Set(state, &sdl_DisplayDev);
}

int Kbd_PollHostKbd(ARMul_State *state)
{
	if (sdlMouseInfo.mouseMoved)
	{
		MouseMoved(state);
	}

	if (sdlMouseInfo.buttonChanged)
	{
		ProcessButton(state);
	}
}

int ChangeRenderResolution(int width, int height)
{
	
	if (PD.cursorTexture) {
		SDL_DestroyTexture(PD.cursorTexture);
	}

	if (PD.displayTexture)
	{
		SDL_DestroyTexture(PD.displayTexture);
	}

	if (!PD.renderer)
	{
		printf("ChangeRenderResolution:Renderer is invalid, can't set new resolution");
	}
	
	PD.cursorTexture = SDL_CreateTexture(PD.renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 32, height);

	PD.displayTexture = SDL_CreateTexture(PD.renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);

	if (!PD.cursorTexture)
	{
		printf("ChangeRenderResolution: %s\n", SDL_GetError());
	}

	if (!PD.displayTexture)
	{
		printf("ChangeRenderResolution: %s\n", SDL_GetError());
	}

	int textureWidth, textureHeight;
	SDL_QueryTexture(PD.displayTexture, NULL, NULL, &textureWidth, &textureHeight);

	printf("Texture: %d %d\n", textureWidth, textureHeight);

	printf("ChangeRenderResolution: changed to: %d, %d\n", width, height);

	PD.width = width;
	PD.height = height;
}