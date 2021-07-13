#include "common.h"
#include "vm/gfx.h"

#include "io/loader.h"
#include "io/stegano.h"
#include "vm/machine.h"
#include "vm/input.h"

#include <cstring>
#include <stdint.h>
#include <future>
#include <SDL/SDL.h>
// libmmenu
#include <dlfcn.h>
#include <mmenu.h>

namespace r8 = retro8;
using pixel_t = uint16_t;

int fps = 60;

//std::future<void> _initFuture;

uint32_t frameCounter = 0;
uint32_t lastFrameTick = 0;

r8::Machine machine;
r8::io::Loader loader;

r8::input::InputManager input;
r8::gfx::ColorTable colorTable;

#ifdef SDL_TRIPLEBUF
#define SDL_FLAGS SDL_HWSURFACE | SDL_TRIPLEBUF
#else
#define SDL_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF
#endif

#ifndef IPU_SCALING
SDL_Surface *real_screen;
#endif
SDL_Surface *sdl_screen;
SDL_Surface *scaled_screen;
static SDL_Rect keepRatio_surface = { 40, 0, 240, 240 };
static SDL_Rect fullscreen_surface = { 0, 0, 320, 240 };
static SDL_Rect game_surface;

void* mmenu;
char* rom_path;

/*
 * MENU ELEMENTS
 */

//
// Font: THIN8X8.pf
// Exported from PixelFontEdit 2.7.0
typedef uint16_t u16;

unsigned char gui_font[2048] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// Char 000 (.)
		0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,	// Char 001 (.)
		0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,	// Char 002 (.)
		0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,	// Char 003 (.)
		0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,	// Char 004 (.)
		0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x7C, 0x38, 0x7C,	// Char 005 (.)
		0x10, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C,	// Char 006 (.)
		0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00,	// Char 007 (.)
		0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,	// Char 008 (.)
		0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,	// Char 009 (.)
		0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,	// Char 010 (.)
		0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,	// Char 011 (.)
		0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,	// Char 012 (.)
		0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0,	// Char 013 (.)
		0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,	// Char 014 (.)
		0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,	// Char 015 (.)
		0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,	// Char 016 (.)
		0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,	// Char 017 (.)
		0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18,	// Char 018 (.)
		0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,	// Char 019 (.)
		0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00,	// Char 020 (.)
		0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0xCC, 0x78,	// Char 021 (.)
		0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00,	// Char 022 (.)
		0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,	// Char 023 (.)
		0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,	// Char 024 (.)
		0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00,	// Char 025 (.)
		0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,	// Char 026 (.) right arrow
		0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00,	// Char 027 (.)
		0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00,	// Char 028 (.)
		0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00,	// Char 029 (.)
		0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00,	// Char 030 (.)
		0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00,	// Char 031 (.)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 032 ( )
		0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x40, 0x00,	// Char 033 (!)
		0x90, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 034 (")
		0x50, 0x50, 0xF8, 0x50, 0xF8, 0x50, 0x50, 0x00,	// Char 035 (#)
		0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0x00,	// Char 036 ($)
		0xC8, 0xC8, 0x10, 0x20, 0x40, 0x98, 0x98, 0x00,	// Char 037 (%)
		0x70, 0x88, 0x50, 0x20, 0x54, 0x88, 0x74, 0x00,	// Char 038 (&)
		0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 039 (')
		0x20, 0x40, 0x80, 0x80, 0x80, 0x40, 0x20, 0x00,	// Char 040 (()
		0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00,	// Char 041 ())
		0x00, 0x20, 0xA8, 0x70, 0x70, 0xA8, 0x20, 0x00,	// Char 042 (*)
		0x00, 0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00,	// Char 043 (+)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x20, 0x40,	// Char 044 (,)
		0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00,	// Char 045 (-)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00,	// Char 046 (.)
		0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,	// Char 047 (/)
		0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00,	// Char 048 (0)
		0x40, 0xC0, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,	// Char 049 (1)
		0x70, 0x88, 0x08, 0x10, 0x20, 0x40, 0xF8, 0x00,	// Char 050 (2)
		0x70, 0x88, 0x08, 0x10, 0x08, 0x88, 0x70, 0x00,	// Char 051 (3)
		0x08, 0x18, 0x28, 0x48, 0xFC, 0x08, 0x08, 0x00,	// Char 052 (4)
		0xF8, 0x80, 0x80, 0xF0, 0x08, 0x88, 0x70, 0x00,	// Char 053 (5)
		0x20, 0x40, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00,	// Char 054 (6)
		0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40, 0x00,	// Char 055 (7)
		0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00,	// Char 056 (8)
		0x70, 0x88, 0x88, 0x78, 0x08, 0x08, 0x70, 0x00,	// Char 057 (9)
		0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00,	// Char 058 (:)
		0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x20,	// Char 059 (;)
		0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x00,	// Char 060 (<)
		0x00, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00,	// Char 061 (=)
		0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80, 0x00,	// Char 062 (>)
		0x78, 0x84, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00,	// Char 063 (?)
		0x70, 0x88, 0x88, 0xA8, 0xB8, 0x80, 0x78, 0x00,	// Char 064 (@)
		0x20, 0x50, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00,	// Char 065 (A)
		0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00,	// Char 066 (B)
		0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00,	// Char 067 (C)
		0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00,	// Char 068 (D)
		0xF8, 0x80, 0x80, 0xE0, 0x80, 0x80, 0xF8, 0x00,	// Char 069 (E)
		0xF8, 0x80, 0x80, 0xE0, 0x80, 0x80, 0x80, 0x00,	// Char 070 (F)
		0x70, 0x88, 0x80, 0x80, 0x98, 0x88, 0x78, 0x00,	// Char 071 (G)
		0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00,	// Char 072 (H)
		0xE0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,	// Char 073 (I)
		0x38, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00,	// Char 074 (J)
		0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00,	// Char 075 (K)
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00,	// Char 076 (L)
		0x82, 0xC6, 0xAA, 0x92, 0x82, 0x82, 0x82, 0x00,	// Char 077 (M)
		0x84, 0xC4, 0xA4, 0x94, 0x8C, 0x84, 0x84, 0x00,	// Char 078 (N)
		0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,	// Char 079 (O)
		0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0x00,	// Char 080 (P)
		0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68, 0x00,	// Char 081 (Q)
		0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88, 0x00,	// Char 082 (R)
		0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70, 0x00,	// Char 083 (S)
		0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,	// Char 084 (T)
		0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,	// Char 085 (U)
		0x88, 0x88, 0x88, 0x50, 0x50, 0x20, 0x20, 0x00,	// Char 086 (V)
		0x82, 0x82, 0x82, 0x82, 0x92, 0x92, 0x6C, 0x00,	// Char 087 (W)
		0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00,	// Char 088 (X)
		0x88, 0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x00,	// Char 089 (Y)
		0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00,	// Char 090 (Z)
		0xE0, 0x80, 0x80, 0x80, 0x80, 0x80, 0xE0, 0x00,	// Char 091 ([)
		0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00,	// Char 092 (\)
		0xE0, 0x20, 0x20, 0x20, 0x20, 0x20, 0xE0, 0x00,	// Char 093 (])
		0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 094 (^)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00,	// Char 095 (_)
		0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 096 (`)
		0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x74, 0x00,	// Char 097 (a)
		0x80, 0x80, 0xB0, 0xC8, 0x88, 0xC8, 0xB0, 0x00,	// Char 098 (b)
		0x00, 0x00, 0x70, 0x88, 0x80, 0x88, 0x70, 0x00,	// Char 099 (c)
		0x08, 0x08, 0x68, 0x98, 0x88, 0x98, 0x68, 0x00,	// Char 100 (d)
		0x00, 0x00, 0x70, 0x88, 0xF8, 0x80, 0x70, 0x00,	// Char 101 (e)
		0x30, 0x48, 0x40, 0xE0, 0x40, 0x40, 0x40, 0x00,	// Char 102 (f)
		0x00, 0x00, 0x34, 0x48, 0x48, 0x38, 0x08, 0x30,	// Char 103 (g)
		0x80, 0x80, 0xB0, 0xC8, 0x88, 0x88, 0x88, 0x00,	// Char 104 (h)
		0x20, 0x00, 0x60, 0x20, 0x20, 0x20, 0x70, 0x00,	// Char 105 (i)
		0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x90, 0x60,	// Char 106 (j)
		0x80, 0x80, 0x88, 0x90, 0xA0, 0xD0, 0x88, 0x00,	// Char 107 (k)
		0xC0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,	// Char 108 (l)
		0x00, 0x00, 0xEC, 0x92, 0x92, 0x92, 0x92, 0x00,	// Char 109 (m)
		0x00, 0x00, 0xB0, 0xC8, 0x88, 0x88, 0x88, 0x00,	// Char 110 (n)
		0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00,	// Char 111 (o)
		0x00, 0x00, 0xB0, 0xC8, 0xC8, 0xB0, 0x80, 0x80,	// Char 112 (p)
		0x00, 0x00, 0x68, 0x98, 0x98, 0x68, 0x08, 0x08,	// Char 113 (q)
		0x00, 0x00, 0xB0, 0xC8, 0x80, 0x80, 0x80, 0x00,	// Char 114 (r)
		0x00, 0x00, 0x78, 0x80, 0x70, 0x08, 0xF0, 0x00,	// Char 115 (s)
		0x40, 0x40, 0xE0, 0x40, 0x40, 0x50, 0x20, 0x00,	// Char 116 (t)
		0x00, 0x00, 0x88, 0x88, 0x88, 0x98, 0x68, 0x00,	// Char 117 (u)
		0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00,	// Char 118 (v)
		0x00, 0x00, 0x82, 0x82, 0x92, 0x92, 0x6C, 0x00,	// Char 119 (w)
		0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00,	// Char 120 (x)
		0x00, 0x00, 0x88, 0x88, 0x98, 0x68, 0x08, 0x70,	// Char 121 (y)
		0x00, 0x00, 0xF8, 0x10, 0x20, 0x40, 0xF8, 0x00,	// Char 122 (z)
		0x10, 0x20, 0x20, 0x40, 0x20, 0x20, 0x10, 0x00,	// Char 123 ({)
		0x40, 0x40, 0x40, 0x00, 0x40, 0x40, 0x40, 0x00,	// Char 124 (|)
		0x40, 0x20, 0x20, 0x10, 0x20, 0x20, 0x40, 0x00,	// Char 125 (})
		0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 126 (~)
		0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00	// Char 127 (.)
		};

SDL_Surface *menuSurface = NULL;

int gui_Fullscreen = 1;
int gui_EnableSound = 0;
int gui_EnableMusic = 1;

static const char *gui_YesNo[2] = { "yes", "no" };

typedef struct {
	const char *itemName;
	int *itemPar;
	int itemParMaxValue;
	const char **itemParName;
	void (*itemOnA)();
} MENUITEM;

typedef struct {
	int itemNum; // number of items
	int itemCur; // current item
	MENUITEM *m; // array of items
} MENU;

void ShowChar(SDL_Surface *s, int x, int y, unsigned char a, int fg_color,
		int bg_color) {
	Uint16 *dst;
	int w, h;

	if (SDL_MUSTLOCK(s))
		SDL_LockSurface(s);
	for (h = 8; h; h--) {
		dst = (Uint16 *) s->pixels + (y + 8 - h) * s->w + x;
		for (w = 8; w; w--) {
			Uint16 color = bg_color; // background
			if ((gui_font[a * 8 + (8 - h)] >> w) & 1)
				color = fg_color; // test bits 876543210
			*dst++ = color;
		}
	}
	if (SDL_MUSTLOCK(s))
		SDL_UnlockSurface(s);
}

void print_string(const char *s, u16 fg_color, u16 bg_color, int x, int y) {
	int i, j = strlen(s);
	for (i = 0; i < j; i++, x += 8)
		ShowChar(menuSurface, x, y, s[i], fg_color, bg_color);
}

void menu_ChangeSoundConfig() {
	if (gui_EnableSound == 0) {
		machine.sound().toggleSound(false);
	} else {
		machine.sound().toggleSound(true);
	}
}

void menu_ChangeMusicConfig() {
	if (gui_EnableSound == 0) {
		machine.sound().toggleMusic(false);
	} else {
		machine.sound().toggleMusic(true);
	}
}

void menu_ChangeScreenConfig() {
	if (gui_Fullscreen == 0) {
		game_surface = fullscreen_surface;
		if (scaled_screen != NULL)
			free(scaled_screen);
		scaled_screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 240,
				sizeof(pixel_t) * 8, 0, 0, 0, 0);
	} else {
		game_surface = keepRatio_surface;
		if (scaled_screen != NULL)
			free(scaled_screen);
		scaled_screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 240, 240,
				sizeof(pixel_t) * 8, 0, 0, 0, 0);
	}
}

MENUITEM gui_MainMenuItems[] = {
		{ (const char *) "Fullscreen: ", &gui_Fullscreen, 1,
				(const char **) &gui_YesNo, &menu_ChangeScreenConfig }, {
				(const char *) "Music: ", &gui_EnableMusic, 1,
				(const char **) &gui_YesNo, &menu_ChangeMusicConfig }, {
				(const char *) "Sound Effect: ", &gui_EnableSound, 1,
				(const char **) &gui_YesNo, &menu_ChangeSoundConfig } };

MENU gui_MainMenu = { 3, 0, (MENUITEM *) &gui_MainMenuItems };

#define color16(red, green, blue) ((red << 11) | (green << 5) | blue)

#define COLOR_BG            color16(05, 03, 02)
#define COLOR_ROM_INFO      color16(22, 36, 26)
#define COLOR_ACTIVE_ITEM   color16(31, 63, 31)
#define COLOR_INACTIVE_ITEM color16(13, 40, 18)
#define COLOR_FRAMESKIP_BAR color16(15, 31, 31)
#define COLOR_HELP_TEXT     color16(16, 40, 24)

void ShowMenuItem(int x, int y, MENUITEM *m, int fg_color) {
	static char i_str[24];

	// if no parameters, show simple menu item
	if (m->itemPar == NULL)
		print_string(m->itemName, fg_color, COLOR_BG, x, y);
	else {
		if (m->itemParName == NULL) {
			// if parameter is a digit
			snprintf(i_str, sizeof(i_str), "%s%i", m->itemName, *m->itemPar);
		} else {
			// if parameter is a name in array
			snprintf(i_str, sizeof(i_str), "%s%s", m->itemName,
					*(m->itemParName + *m->itemPar));
		}
		print_string(i_str, fg_color, COLOR_BG, x, y);
	}
}

/*
 Shows menu items and pointing arrow
 */
void menu_ShowMenu(MENU *menu) {

	int i;
	MENUITEM *mi = menu->m;

	// clear buffer
	SDL_FillRect(menuSurface, NULL, COLOR_BG);

	// show menu lines
	for (i = 0; i < menu->itemNum; i++, mi++) {
		int fg_color;
		if (menu->itemCur == i)
			fg_color = COLOR_ACTIVE_ITEM;
		else
			fg_color = COLOR_INACTIVE_ITEM;
		ShowMenuItem(80, (8 + i) * 10, mi, fg_color);
	}

	print_string("Retro8 for Trimui : " __DATE__ " build", COLOR_HELP_TEXT,
			COLOR_BG, 5, 2);
	print_string("[B] = Return to game", COLOR_HELP_TEXT, COLOR_BG, 5, 210);
	print_string("Port by Gameblabla", COLOR_HELP_TEXT, COLOR_BG, 5, 220);
	print_string("Trimui version by Liartes", COLOR_HELP_TEXT, COLOR_BG, 5,
			230);

}

/*
 Main function that runs all the stuff
 */

void gui_Flip() {
	if (real_screen->w == 320 || real_screen->w == 240) {
		SDL_BlitSurface(menuSurface, NULL, real_screen, NULL);
	} else {
		SDL_SoftStretch(menuSurface, NULL, real_screen, NULL);
	}
	SDL_Flip(real_screen);
}

void menu_MainShow(MENU *menu) {
	SDL_Event gui_event;
	MENUITEM *mi;

	bool done = false;

	if (menuSurface == NULL)
		menuSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0,
				0);

	while (!done) {
		mi = menu->m + menu->itemCur; // pointer to highlit menu option

		while (SDL_PollEvent(&gui_event)) {
			puts("MENU SDL_POLLEVENT LOOP");
			if (gui_event.type == SDL_KEYDOWN) {
				puts("VALIDATION MENU ITEM BEGIN");
				if (gui_event.key.keysym.sym == SDLK_SPACE)
					if (mi->itemOnA != NULL)
						mi->itemOnA(); // TRIMUI A
				puts("VALIDATION MENU ITEM END");
				if (gui_event.key.keysym.sym == SDLK_LCTRL)
					return; // TRIMUI B

				if (gui_event.key.keysym.sym == SDLK_UP)
					if (--menu->itemCur < 0)
						menu->itemCur = menu->itemNum - 1;

				if (gui_event.key.keysym.sym == SDLK_DOWN)
					if (++menu->itemCur == menu->itemNum)
						menu->itemCur = 0;

				if (gui_event.key.keysym.sym == SDLK_LEFT) {
					if (mi->itemPar != NULL && *mi->itemPar > 0)
						*mi->itemPar -= 1;
				}
				if (gui_event.key.keysym.sym == SDLK_RIGHT) {
					if (mi->itemPar != NULL
							&& *mi->itemPar < mi->itemParMaxValue)
						*mi->itemPar += 1;
				}
			}
		}
		if (!done)
			menu_ShowMenu(menu); // show menu items
		SDL_Delay(16);
		gui_Flip();
	}
	SDL_FillRect(menuSurface, NULL, 0);
	SDL_Flip(menuSurface);
	SDL_FillRect(menuSurface, NULL, 0);
	SDL_Flip(menuSurface);
#ifdef SDL_TRIPLEBUF
	SDL_FillRect(menuSurface, NULL, 0);
	SDL_Flip(menuSurface);
#endif
}

/*
 * END MENU ELEMENTS
 */

struct ColorMapper {
	r8::gfx::ColorTable::pixel_t operator()(uint8_t r, uint8_t g,
			uint8_t b) const {
		return SDL_MapRGB(sdl_screen->format, r, g, b);
	}
};

uint32_t Platform::getTicks() {
	return SDL_GetTicks();
}

void deinit() {
	if (sdl_screen)
		SDL_FreeSurface(sdl_screen);
	if (scaled_screen)
		SDL_FreeSurface(scaled_screen);
#ifndef IPU_SCALING
	if (real_screen)
		SDL_FreeSurface(real_screen);
#endif
	//TODO: release all structures bound to Lua etc
}

void flip_screen() {
	fps = machine.code().require60fps() ? 60 : 30;
	auto* data = machine.memory().screenData();
	auto* screenPalette = machine.memory().paletteAt(
			r8::gfx::SCREEN_PALETTE_INDEX);
	auto output = static_cast<pixel_t*>(sdl_screen->pixels);

	for (size_t i = 0; i < r8::gfx::BYTES_PER_SCREEN; ++i) {
		const r8::gfx::color_byte_t* pixels = data + i;
		const auto rc1 = colorTable.get(screenPalette->get((pixels)->low()));
		const auto rc2 = colorTable.get(screenPalette->get((pixels)->high()));

		*(output) = rc1;
		*((output) + 1) = rc2;
		(output) += 2;
	}

#ifndef IPU_SCALING
	SDL_SoftStretch(sdl_screen, NULL, scaled_screen, NULL);
	SDL_BlitSurface(scaled_screen, NULL, real_screen, &game_surface);
	SDL_Flip(real_screen);
#else
	SDL_Flip(sdl_screen);
#endif
	++frameCounter;

	/* Of course this assumes that the actual screen refresh rate is 60 or close to it.
	 * Most games are 30 FPS though. I'm not aware of any 60 fps games aside from one
	 * which only works on Picolove. - Gameblabla */
#ifdef SDL_TRIPLEBUF
	if (fps != 60) {
#endif
	//SDL_Delay(1000.0/fps);
	uint32_t now = SDL_GetTicks();
	if (now - lastFrameTick < 1000.0 / fps)
		SDL_Delay(1000.0 / fps - (now - lastFrameTick));
	lastFrameTick = SDL_GetTicks();
	//while((1000/fps) > SDL_GetTicks()-start)SDL_Delay((1000/fps)-(SDL_GetTicks()-start));
#ifdef SDL_TRIPLEBUF
}
#endif
}


bool load_game(char* rom_name) {
	size_t sz = 0;
	FILE* fp;
	uint8_t* bdata;

	input.reset();
	machine.setflip(flip_screen);
	machine.sound().init();
	menu_ChangeMusicConfig();
	frameCounter = 0;

	if (strstr(rom_name, ".PNG") || strstr(rom_name, ".png")) {
		fp = fopen(rom_name, "rb");
		if (!fp)
			return false;

		fseek(fp, 0, SEEK_END);
		sz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		bdata = (uint8_t*) malloc(sz);
		fread(bdata, sz, 1, fp);
		fclose(fp);

		std::vector < uint8_t > out;
		unsigned long width, height;
		auto result = Platform::loadPNG(out, width, height, (uint8_t*) bdata,
				sz, true);
		assert(result == 0);

		if (bdata)
			free(bdata);

		r8::io::Stegano stegano;
		stegano.load(
				{ reinterpret_cast<const uint32_t*>(out.data()), nullptr,
						out.size() / 4 }, machine);
	} else {
		//TODO: not efficient since it's copied and it's not checking for '\0'
		std::string raw(rom_name);
		r8::io::Loader loader;
		loader.loadFile(raw, machine);
	}

	machine.memory().backupCartridge();

	machine.code().init();

	return true;
}

void audio_callback(void* data, Uint8* cbuffer, int length) {

	retro8::sfx::APU* apu = static_cast<retro8::sfx::APU*>(data);

	int16_t* buffer = reinterpret_cast<int16_t*>(cbuffer);

	apu->renderSounds(buffer, length / sizeof(int16_t));
	return;
}

uint_fast8_t retro_run() {
	Uint32 start;
	SDL_Event Event;

	/* manage input */
	while (SDL_PollEvent(&Event)) {
		switch (Event.key.keysym.sym) {
		case SDLK_LEFT:
			input.manageKey(0, 0, Event.type == SDL_KEYDOWN);
			break;
		case SDLK_RIGHT:
			input.manageKey(0, 1, Event.type == SDL_KEYDOWN);
			break;
		case SDLK_UP:
			input.manageKey(0, 2, Event.type == SDL_KEYDOWN);
			break;
		case SDLK_DOWN:
			input.manageKey(0, 3, Event.type == SDL_KEYDOWN);
			break;

		case SDLK_z:
		case SDLK_LCTRL: // Trimui B
			input.manageKey(0, 4, Event.type == SDL_KEYDOWN);
			break;

		case SDLK_x:
		case SDLK_SPACE: // Trimui A
			input.manageKey(0, 5, Event.type == SDL_KEYDOWN);
			break;

		case SDLK_a:
		case SDLK_LALT: // Trimui X
			input.manageKey(1, 4, Event.type == SDL_KEYDOWN);
			break;

		case SDLK_s:
		case SDLK_LSHIFT: // Trimui Y
			input.manageKey(1, 5, Event.type == SDL_KEYDOWN);
			break;
		case SDLK_ESCAPE:
			mmenu = dlopen("libmmenu.so", RTLD_LAZY);

			if (mmenu) {
				ShowMenu_t ShowMenu = (ShowMenu_t) dlsym(mmenu, "ShowMenu");

				SDL_PauseAudio(1);
				MenuReturnStatus status = ShowMenu(rom_path, NULL, real_screen,
						kMenuEventKeyDown);

				if (status == kStatusExitGame) {
					SDL_FillRect(real_screen, NULL, 0x000000);
					SDL_Flip(real_screen);
					return 0;
				} else if (status == kStatusOpenMenu) {
					menu_MainShow(&gui_MainMenu);
					SDL_FillRect(real_screen, NULL, 0x000000);
					SDL_Flip(real_screen);
				}
				SDL_PauseAudio(0);
			} else {
				menu_MainShow(&gui_MainMenu);
				SDL_FillRect(real_screen, NULL, 0x000000);
				SDL_Flip(real_screen);
			}

			break;
		default:
			break;
		}
	}

	input.tick();

	machine.code().update();
	machine.code().draw();

	flip_screen();
	input.manageKeyRepeat();

	return 1;
}

int main(int argc, char* argv[]) {
	int res = 0, while_res = 1;
	if (gui_Fullscreen == 1) {
		game_surface = keepRatio_surface;
	} else {
		game_surface = fullscreen_surface;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_ShowCursor(0);
#ifdef IPU_SCALING
	sdl_screen = SDL_SetVideoMode(128, 128, sizeof(pixel_t) * 8, SDL_FLAGS);
#else
	sdl_screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 128, 128,
			sizeof(pixel_t) * 8, 0, 0, 0, 0);
	if (gui_Fullscreen == 1) {
		scaled_screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 240, 240,
				sizeof(pixel_t) * 8, 0, 0, 0, 0);
	} else {
		scaled_screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 240,
				sizeof(pixel_t) * 8, 0, 0, 0, 0);
	}
	real_screen = SDL_SetVideoMode(320, 240, sizeof(pixel_t) * 8, SDL_FLAGS);
	if (!real_screen) {
		real_screen = SDL_SetVideoMode(240, 160, sizeof(pixel_t) * 8,
				SDL_FLAGS);
		if (!real_screen) {
			return 0;
		}
	}
#endif

	SDL_AudioSpec wantSpec, spec;
	wantSpec.freq = 44100;
	wantSpec.format = AUDIO_S16SYS;
	wantSpec.channels = 1;
	wantSpec.samples = 2048;
	wantSpec.userdata = &machine.sound();
	wantSpec.callback = audio_callback;

	SDL_OpenAudio(&wantSpec, &spec);
	SDL_PauseAudio(0);

	colorTable.init(ColorMapper());
	machine.font().load();
	machine.code().loadAPI();
	input.setMachine(&machine);

	if (argc != 2) {
		printf("Usage: %s GAME_ROM\n", argv[0]);
		return 1;
	}
	rom_path = argv[1];
	res = load_game(rom_path);
	if (!res) {
		printf("Could not load game '%s'!\n", argv[1]);
		return 0;
	}

	while (while_res) {
		while_res = retro_run();
	}

	SDL_PauseAudio(1);
	deinit();
	SDL_Quit();

	return 0;
}

