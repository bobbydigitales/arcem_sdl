/* (c) David Alan Gilbert 1995-1999 - see Readme file for copying info
   Hacked about with for new display driver interface by Jeffrey Lee, 2011 */
/* Display and keyboard interface for the Arc emulator */

/*#define DEBUG_VIDCREGS */
#define DEBUG_KBD
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

#include "../armemu.h"

#include "ControlPane.h"
#include "KeyTable.h"
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
	"0,0"};

struct ArcKeyTrans transTable[]={
{SDLK_ESCAPE, 0, 0}, {SDLK_F1,  0, 1}, {SDLK_F2,  0, 2}, {SDLK_F3,  0, 3},
{SDLK_F4,     0, 4}, {SDLK_F5,  0, 5}, {SDLK_F6,  0, 6}, {SDLK_F7,  0, 7},
{SDLK_F8,     0, 8}, {SDLK_F9,  0, 9}, {SDLK_F10, 0,10}, {SDLK_F11, 0,11},
{SDLK_F12,    0,12}, {SDLK_PRINTSCREEN,0,13},{SDLK_SCROLLLOCK,0,14}, {SDLK_PAUSE,0,15},/*{XK_Break,0,15}, */

{SDLK_BACKQUOTE, 1, 0},{SDLK_1, 1, 1},{SDLK_2, 1, 2},{SDLK_3, 1, 3},
{SDLK_4, 1, 4}, {SDLK_5, 1, 5}, {SDLK_6, 1, 6}, {SDLK_7, 1, 7},
{SDLK_8, 1, 8}, {SDLK_9, 1, 9},{SDLK_0, 1, 10}, {SDLK_MINUS, 1, 11},
{SDLK_PLUS, 1, 12}, /*{POUND, 1, 13},*/ {SDLK_BACKSPACE, 1, 14},{SDLK_INSERT, 1, 15},

{SDLK_HOME, 2, 0}, {SDLK_PAGEUP, 2, 1}, {SDLK_NUMLOCKCLEAR, 2, 2}, {SDLK_KP_DIVIDE, 2, 3},
{SDLK_KP_MULTIPLY, 2, 4}, /*{KP_HASH, 2, 5}*/ {SDLK_TAB, 2, 6}, {SDLK_q, 2, 7},
{SDLK_w, 2, 8}, {SDLK_e, 2, 9}, {SDLK_r, 2, 10}, {SDLK_t, 2, 11},
{SDLK_y,2,12}, {SDLK_u, 2, 13}, {SDLK_i, 2, 14}, {SDLK_o, 2, 15},

{SDLK_p, 3, 0}, {SDLK_LEFTPAREN, 3, 1}, {SDLK_RIGHTPAREN,3,2}, {SDLK_BACKSLASH, 3, 3},
{SDLK_DELETE, 3, 4}, {SDLK_END, 3, 5}, {SDLK_PAGEDOWN, 3, 6}, {SDLK_KP_7, 3, 7},
{SDLK_KP_8, 3, 8}, {SDLK_KP_9, 3, 9}, {SDLK_KP_MINUS, 3, 10}, {SDLK_LCTRL, 3, 11},
{SDLK_a, 3, 12}, {SDLK_s, 3, 13}, {SDLK_d, 3, 14}, {SDLK_f, 3, 15},

{SDLK_g, 4, 0}, {SDLK_h, 4, 1}, {SDLK_j, 4, 2}, {SDLK_k, 4, 3},
{SDLK_l, 4, 4}, {SDLK_SEMICOLON, 4, 5}, {SDLK_QUOTE, 4, 6}, {SDLK_RETURN, 4, 7},
{SDLK_KP_4, 4, 8}, {SDLK_KP_5, 4, 9}, {SDLK_KP_6, 4, 10}, {SDLK_PLUS, 4, 11},
{SDLK_LSHIFT, 4, 12}, /*{??, 4, 13},*/{SDLK_z, 4, 14}, {SDLK_x, 4, 15},


{SDLK_c, 5, 0}, {SDLK_v, 5, 1}, {SDLK_b, 5, 2}, {SDLK_n, 5, 3},
{SDLK_m, 5, 4}, {SDLK_COMMA, 5, 5}, {SDLK_PERIOD, 5, 6}, {SDLK_SLASH, 5, 7},
{SDLK_RSHIFT, 5, 8}, {SDLK_UP,5,9}, {SDLK_KP_1, 5, 10}, {SDLK_KP_2, 5, 11},
{SDLK_KP_3, 5, 12}, {SDLK_CAPSLOCK, 5, 13}, {SDLK_LALT, 5, 14}, {SDLK_SPACE, 5, 15},

{SDLK_RALT, 6, 0}, {SDLK_RCTRL, 6, 1}, {SDLK_LEFT, 6, 2}, {SDLK_DOWN, 6, 3},
{SDLK_RIGHT, 6, 4}, {SDLK_KP_0, 6, 5}, {SDLK_KP_PERIOD, 6, 6}, {SDLK_KP_ENTER, 6, 7},

{0,-1,-1} /* Termination of list */
};


struct keyloc invertedKeyTable[0xff];

/*-----------------------------------------------------------------------------
 * GenerateInvertedKeyTable - Turns the list of (symbol, row, col) tuples into
 * a list of just (row, col) tuples that are ordered by sym. This makes look
 * ups in ProcessKey much simpler. Invalid entries will have (-1, -1).
 */
static void GenerateInvertedKeyTable()
{
    // Find out how many entries we have
    int i;

    memset(invertedKeyTable, 0xff, sizeof(invertedKeyTable));

    // for each inverted entry...
    for (i = 0; i < 0xff; i++)
    {
        struct ArcKeyTrans *PresPtr;
        printf("Keymap : %x\t", i);
        // find the keymap
        for(PresPtr = transTable; PresPtr->row != -1; PresPtr++)
        {
            if (PresPtr->sym == i ||  PresPtr->sym - 1073741752 == i)
            {
                // Found a match
                invertedKeyTable[i].row = PresPtr->row;
                invertedKeyTable[i].col = PresPtr->col;
                printf(" %d, \t %d", PresPtr->row, PresPtr->col);
                break;
            }
        }
    printf("\n");   
    }
}



static void ProcessKey(ARMul_State *state, SDL_Event event);

static SDL_Cursor *init_system_cursor(const char *image[])
{
	int i, row, col;
	Uint8 data[4 * 32];
	Uint8 mask[4 * 32];
	int hot_x, hot_y;

	i = -1;
	for (row = 0; row < 32; ++row)
	{
		for (col = 0; col < 32; ++col)
		{
			if (col % 8)
			{
				data[i] <<= 1;
				mask[i] <<= 1;
			}
			else
			{
				++i;
				data[i] = mask[i] = 0;
			}
			switch (image[4 + row][col])
			{
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
	sscanf(image[4 + row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

int ChangeRenderResolution(int width, int height);

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

	if (sdlMouseInfo.buttonIndex)

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

	PD.window = SDL_CreateWindow("ArcEm:SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640,	480, SDL_WINDOW_RESIZABLE);

	if (!PD.window)
	{
		printf("DisplayDev_Init:Couldn't create window :(");
		exit(1);
	}

	PD.renderer = SDL_CreateRenderer(PD.window, -1, SDL_RENDERER_PRESENTVSYNC);


		// SDL_CreateWindowAndRenderer(initialWindowWidth, initialWindowHeight, SDL_WINDOW_RESIZABLE, &PD.window, &PD.renderer);
	// SDL_SetWindowTitle(PD.window, "ArcEm");
	ChangeRenderResolution(initialWindowWidth, initialWindowHeight);
	SDL_GL_SetSwapInterval(1);
	// PD.cursor = init_system_cursor(arrow);
	// SDL_SetCursor(PD.cursor);
	// SDL_ShowCursor(SDL_ENABLE);


	GenerateInvertedKeyTable();

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

	SDL_DestroyTexture(PD.cursorTexture);
	SDL_DestroyTexture(PD.displayTexture);

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

//  SDD_HostColour
//  - Data type used to store a colour value. E.g.
//    "typedef unsigned short SDD_HostColour".
typedef uint32_t SDD_HostColour;

// SDD_Name(x)
//  - Macro used to convert symbol name 'x' into an instance-specific version
//    of the symbol. e.g.
//    "#define SDD_Name(x) MySDD_##x"

#define SDD_Name(x) sdl_##x
// #define SDD_Stats TRUE
// #define VIDEO_STATS TRUE

// SDD_RowsAtOnce
//  - The number of source rows to process per update. This can be a non-const
//    variable if you want, so it can be tweaked on the fly to tune performance
static const int SDD_RowsAtOnce = 1;

// SDD_Row
//  - A data type that acts as an iterator/accessor for a screen row. It can be
//    anything from a simple *SDD_HostColour (for hosts with direct framebuffer
//    access, e.g. RISC OS), or something more complex like a struct which keeps
//    track of the current drawing coordinates (e.g. the truecolour X11 driver)
//  - SDD_Row instances must be able to cope with being passed by value as
//    function parameters.
typedef SDD_HostColour *SDD_Row;

// uint32_t pixelFromRGB(int red, int green, int blue)
// {
//    return red << 16 | green << 8 | blue;
// }

void putpixel(SDD_HostColour *pixels, int x, int y, uint32_t pixel)
{
	if (!pixels)
	{
		return;
	}

	pixels[(y * PD.cursorPitch / 4) + x] = pixel;
}

int count = 0;

void RefreshMouse(ARMul_State *state);

int ChangeRenderResolution(int width, int height);



//  - A function that the driver will call at the start of each frame.
void SDD_Name(Host_PollDisplay)(ARMul_State *state)
{
	//printf("Hello!");

	// printf("Host_PollDisplay\n");

	SDL_Event event;

	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{

		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}

			ProcessKey(state, event);
			break;
		case SDL_KEYUP:

			ProcessKey(state, event);
			break;


		case SDL_QUIT:
			SDL_DestroyRenderer(PD.renderer);
			SDL_DestroyWindow(PD.window);
			SDL_DestroyTexture(PD.displayTexture);
			SDL_Quit();
			exit(0);
			break;

		case SDL_MOUSEMOTION:
			printf("Mouse motion: %d, %d\n", event.motion.xrel, event.motion.yrel);
			sdlMouseInfo.xDiff = event.motion.xrel;
			sdlMouseInfo.yDiff = event.motion.yrel;
			sdlMouseInfo.mouseMoved = true;
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			SDL_SetRelativeMouseMode(SDL_TRUE);
			sdlMouseInfo.buttonIndex = event.button.button;
			sdlMouseInfo.UpNDown = (event.button.state == SDL_RELEASED) ? true : false;
			sdlMouseInfo.buttonChanged = true;
			printf("Mouse button event: %d, %d\n", sdlMouseInfo.buttonIndex, sdlMouseInfo.UpNDown);
			break;

		default:
			break;
		}
	}

	RefreshMouse(state);

	if (PD.window && PD.renderer && PD.displayTexture && PD.pixels && PD.cursorPixels)
	{
		// SDL_SetRenderDrawColor(PD.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		// SDL_RenderClear(PD.renderer);
		SDL_UnlockTexture(PD.displayTexture);
		SDL_RenderCopy(PD.renderer, PD.displayTexture, NULL, NULL);

		SDL_Rect srcRect, dstRect;

		srcRect.x = 0;
		srcRect.y = 0;
		srcRect.w = 32;
		srcRect.h = PD.mouseHeight;

		dstRect.x = PD.mouseX;
		dstRect.y = PD.mouseY;
		dstRect.w = 32;
		dstRect.h = PD.mouseHeight;

		SDL_UnlockTexture(PD.cursorTexture);
		SDL_RenderCopy(PD.renderer, PD.cursorTexture, &srcRect, &dstRect);

		SDL_RenderPresent(PD.renderer);
	}

	if (SDL_LockTexture(PD.displayTexture, NULL, &PD.pixels, &PD.pitch) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock display texture: %s\n", SDL_GetError());
	}

	if (SDL_LockTexture(PD.cursorTexture, NULL, &PD.cursorPixels, &PD.cursorPitch) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock cursor texture: %s\n", SDL_GetError());
	}
}

// SDD_HostColour SDD_Name(Host_GetColour)(ARMul_State *state,uint_fast16_t col)
//  - A function that the driver will call in order to convert a 13-bit VIDC
//    physical colour into a host colour.
SDD_HostColour SDD_Name(Host_GetColour)(ARMul_State *state, uint_fast16_t col)
{
	// I think the 13-bit VIDC values are like this:
	// ? bbbb gggg rrrr
	// and we need to convert to 8-bit RGB values, so we mask off each component, then
	// shift them left by 4 positions.
	int r = (col & 0x00f) << 4;
	int g = (col & 0x0f0);
	int b = (col & 0xf00) >> 4;
	//    printf("Host_GetColour: %d %d %d\n", r, g, b);

	return r << 16 | g << 8 | b;
}


static void ProcessKey(ARMul_State *state, SDL_Event event)
{
int key;


if (KBD.BuffOcc >= KBDBUFFLEN)
	{
#ifdef DEBUG_KBD
		fprintf(stderr, "KBD: Missed mouse event - buffer full\n");
#endif
		return;
}

if (event.key.keysym.sym > 0x7f)
	{
	key = event.key.keysym.sym - 1073741752;
#ifdef DEBUG_KBD
		
	fprintf(stderr, "KBD: mapped sym = %d, key= %0x \n",
			event.key.keysym.sym, key );
#endif	
}
else
	{
	key = event.key.keysym.sym;
}


if(key > 0xff){
#ifdef DEBUG_KBD
		
	fprintf(stderr, "KBD: Unkown key sym = %d, %0x \n",
			event.key.keysym.sym, key );
#endif
	return;
}

if (invertedKeyTable[key].row ==-1)
	{

#ifdef DEBUG_KBD
	fprintf(stderr, "KBD: Unmapped key sym = %d , key = %0x \n",
			event.key.keysym.sym, key);
#endif

	return;
	} else 
	{

	KBD.Buffer[KBD.BuffOcc].KeyColToSend = invertedKeyTable[key].col;
	KBD.Buffer[KBD.BuffOcc].KeyRowToSend = invertedKeyTable[key].row;
	KBD.Buffer[KBD.BuffOcc].KeyUpNDown = (event.key.type == SDL_KEYUP) ? true : false;
	KBD.BuffOcc++;
#ifdef DEBUG_KBD
    fprintf(stderr,"ProcessKey: Got Col,Row=%d,%d UpNDown=%d BuffOcc=%d\n",
              KBD.Buffer[KBD.BuffOcc].KeyColToSend,
              KBD.Buffer[KBD.BuffOcc].KeyRowToSend,
              KBD.Buffer[KBD.BuffOcc].KeyUpNDown,
              KBD.BuffOcc);
#endif


	
	}



}


// SDD_Row SDD_Name(Host_BeginRow)(ARMul_State *state,int row,int offset)
//  - Function to return a SDD_Row instance suitable for accessing the indicated
//    row, starting from the given X offset
SDD_Row SDD_Name(Host_BeginRow)(ARMul_State *state, int row, int offset)
{
	// printf("Host_BeginRow, ");
	return (uint32_t *)((uint8_t *)PD.pixels + row * PD.pitch);
}

// void SDD_Name(Host_EndRow)(ARMul_State *state,SDD_Row *row)
//  - Function to end the use of a SDD_Row
//  - Where a SDD_Row has been copied via pass-by-value, currently only the
//    original instance will have Host_EndRow called on it.
void SDD_Name(Host_EndRow)(ARMul_State *state, SDD_Row *row)
{
	// printf("Host_EndRow, ");
	// Nothing here
}

// void SDD_Name(Host_BeginUpdate)(ARMul_State *state,SDD_Row *row,
//                                 unsigned int count)
//  - Function called when the driver is about to write to 'count' pixels of the
//    row. Implementations could use it for tracking dirty regions in the
//    display window.
void SDD_Name(Host_BeginUpdate)(ARMul_State *state, SDD_Row *row, unsigned int count)
{
	// printf("Host_BeginUpdate, ");
}
// void SDD_Name(Host_EndUpdate)(ARMul_State *state,SDD_Row *row)
//  - Function called once the driver has finished the write operation
void SDD_Name(Host_EndUpdate)(ARMul_State *state, SDD_Row *row)
{
	// printf("Host_EndUpdate, ");
}
// void SDD_Name(Host_SkipPixels)(ARMul_State *state,SDD_Row *row,
//                                unsigned int count)
//  - Function to skip forwards 'count' pixels in the row
void SDD_Name(Host_SkipPixels)(ARMul_State *state, SDD_Row *row, unsigned int count)
{
	// printf("Host_SkipPixels, ");
	(*row) += count;
}

// void SDD_Name(Host_WritePixel)(ARMul_State *state,SDD_Row *row,
//                                SDD_HostColour col)
//  - Function to write a single pixel and advance to the next location. Only
//    called between BeginUpdate & EndUpdate.
void SDD_Name(Host_WritePixel)(ARMul_State *state, SDD_Row *row, SDD_HostColour pix)
{
	// printf("WP, ");
	*(*row)++ = pix;
}

// void SDD_Name(Host_WritePixels)(ARMul_State *state,SDD_Row *row,
//                                 SDD_HostColour col,unsigned int count)
//  - Function to fill N adjacent pixels with the same colour. 'count' may be
//    zero. Only called between BeginUpdate & EndUpdate.
void SDD_Name(Host_WritePixels)(ARMul_State *state, SDD_Row *row, SDD_HostColour pix, unsigned int count)
{
	// printf("WPS, ");
	while (count--)
	{
		*(*row)++ = pix;
	}
}

// SDD_Stats
//  - Define this to enable the stats code.
//#define SDD_Stats

// SDD_DisplayDev
//  - The name to use for the const DisplayDev struct that will be generated
#define SDD_DisplayDev sdl_DisplayDev

static void SDD_Name(Host_ChangeMode)(ARMul_State *state, int width, int height, int hz);

#include "../arch/stddisplaydev.c"

// void SDD_Name(Host_ChangeMode)(ARMul_State *state,int width,int height,
//                                int hz)
//  - A function that the driver will call whenever the display timings have
//    changed enough to warrant a mode change.
//  - The implementation must change to the most appropriate display mode
//    available and fill in the Width, Height, XScale and YScale members of the
//    HostDisplay struct to reflect the new display parameters.
//  - Currently, failure to find a suitable mode isn't supported - however it
//    shouldn't be too hard to keep the screen blanked (not ideal) by keeping
//    DC.ModeChanged set to 1
void SDD_Name(Host_ChangeMode)(ARMul_State *state, int width, int height, int hz)
{
	HD.Width = width;
	HD.Height = height;
	HD.XScale = 1;
	HD.YScale = 1;
	/* Try and detect rectangular pixel modes */
	if ((width >= height * 2))
	{
		HD.YScale = 2;
		HD.Height *= 2;
	}
	else if ((height >= width))
	{
		HD.XScale = 2;
		HD.Width *= 2;
	}

	ChangeRenderResolution(HD.Width, HD.Height);
	// resizeWindow(HD.Width, HD.Height);
	// /* Screen is expected to be cleared */
	// memset(dibbmp, 0, sizeof(SDD_HostColour) * MonitorWidth * MonitorHeight);

	printf("Host_ChangeMode, ");
}

void RefreshMouse(ARMul_State *state)
{

	if (!PD.cursorPixels)
	{
		return;
	}

	int x, y, offset, pix, repeat;
	int memptr;
	int HorizPos;
	int Height = ((int)VIDC.Vert_CursorEnd - (int)VIDC.Vert_CursorStart) * HD.YScale;
	int VertPos;
	int textureOffset;
	SDD_HostColour cursorPal[4];

	DisplayDev_GetCursorPos(state, &HorizPos, &VertPos);
	HorizPos = HorizPos * HD.XScale + HD.XOffset;
	VertPos = VertPos * HD.YScale + HD.YOffset;

	if (Height < 0)
		Height = 0;
	if (VertPos < 0)
		VertPos = 0;

	PD.mouseX = HorizPos;
	PD.mouseY = VertPos;
	PD.mouseHeight = Height;

	//    printf("HorizPos: %d, VertPos: %d, Height: %d\n", HorizPos, VertPos, Height);

	/* Cursor palette */
	cursorPal[0] = 0;
	for (x = 0; x < 3; x++)
	{
		cursorPal[x + 1] = SDD_Name(Host_GetColour)(state, VIDC.CursorPalette[x]);
		// printf("palette colour: %d\n", cursorPal[x + 1]);
	}

	offset = 0;
	memptr = MEMC.Cinit * 16;
	repeat = 0;
	for (y = 0; y < Height; y++)
	{
		if (offset < 512 * 1024)
		{
			ARMword tmp[2];

			tmp[0] = MEMC.PhysRam[memptr / 4];
			tmp[1] = MEMC.PhysRam[memptr / 4 + 1];

			for (x = 0; x < 32; x++)
			{
				pix = ((tmp[x / 16] >> ((x & 15) * 2)) & 3);

				textureOffset = PD.mouseX + x + (PD.height - PD.mouseY - y - 1) * PD.width;

				// printf("p:%d o:%d ", pix, textureOffset);

				putpixel(PD.cursorPixels, x, y, cursorPal[pix]);

				// curbmp[x + (MonitorHeight - y - 1) * 32] =
				//     (pix || diboffs < 0) ? cursorPal[pix] : dibbmp[diboffs];
			};
		}
		// else
		//    return;
		// if (++repeat == HD.YScale)
		// {
		//    memptr += 8;
		//    offset += 8;
		//    repeat = 0;
		// }
	};

	// printf("\n");
}
