// Author: Cyle Riggs (@beardedfoo)
// This program for the Sega Genesis/Megadrive demonstrates the
// use of PLAN_A and PLAN_B as well as palette transparency to
// place a movable, windowed cursor on screen.
#include <genesis.h>

// Disable the use of DMA for tile loading
#define LOAD_TILE_WITH_DMA FALSE

// Load one tile at a time into the VDP
#define LOAD_TILE_COUNT 1

// Do not allow the cursor to extend beyond the visible plane
#define X_MIN 0
#define X_MAX 39
#define Y_MIN 0
#define Y_MAX 27

// Define TILE_A as a solid block of color from palette entry 1
#define VRAM_POS_TILE_FILL 1
const u32 TILE_FILL[8] =
{
  0x11111111,
  0x11111111,
  0x11111111,
  0x11111111,
  0x11111111,
  0x11111111,
  0x11111111,
  0x11111111,
};

// Define TILE_CURSOR as an empty square; a tranparent inner square
// and an outer square of color 2 from the palette. Since this tile
// will be placed in front of TILE_FILL the pixels with color 0 will
// be transparent and color from TILE_FILL will be visible in these
// locations. Note how the shape of the drawn image is visible in
// the arrangement of 2's and 0's in the tile data, if stretched.
#define VRAM_POS_TILE_CURSOR 2
const u32 TILE_CURSOR[8] =
{
  0x22222222,
  0x20000002,
  0x20000002,
  0x20000002,
  0x20000002,
  0x20000002,
  0x20000002,
  0x22222222,
};

// Define a basic color palette
const u16 palette_basic[16] = {
  0x000, // color 0 = black
  0xEEE, // color 1 = white
  0xE0E, // color 2 = magenta

  // Rest of the palette is unused in this application
  0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
  0x000, 0x000, 0x000, 0x000,
};

// Use a variable to instruct the main loop how to move
#define DIR_RIGHT 1
#define DIR_LEFT 2
#define DIR_UP 3
#define DIR_DOWN 4
u16 move_dir = NULL;

void joyHandler(u16 joy, u16 changed, u16 state) {
  // Ignore input from anything except player one
  if (joy != JOY_1) {
    return;
  }

  // Move the cursor with gamepad input
  if (state & BUTTON_LEFT) {
    move_dir = DIR_LEFT;
  } else if (state & BUTTON_RIGHT) {
    move_dir = DIR_RIGHT;
  } else if (state & BUTTON_UP) {
    move_dir = DIR_UP;
  } else if (state & BUTTON_DOWN) {
    move_dir = DIR_DOWN;
  }
}

// Draw the background in PLAN_A and the cursor in PLAN_B, which
// has a higher drawing priority and allows for transparency
// effects on the tiles.
#define CURSOR_PLANE PLAN_A
#define BG_PLANE PLAN_B

int main()
{
  // Place the cursor on a grid of 40x28 tiles
  u16 cursor_x = 20;
  u16 cursor_y = 14;

  // Setup the video chip
  VDP_init();

  // Setup gamepad input handling
  JOY_init();
  JOY_setEventHandler(&joyHandler);

  // Load the tile data into the video chip (one at a time)
  VDP_loadTileData(TILE_FILL, VRAM_POS_TILE_FILL, LOAD_TILE_COUNT, LOAD_TILE_WITH_DMA);
  VDP_loadTileData(TILE_CURSOR, VRAM_POS_TILE_CURSOR, LOAD_TILE_COUNT, LOAD_TILE_WITH_DMA);

  // Load the color palette as palette 0
  VDP_setPalette(PAL0, palette_basic);

  // Fill PLAN_B with TILE_FILL
  for (u16 fill_x = X_MIN; fill_x <= X_MAX; fill_x++) {
    for (u16 fill_y = Y_MIN; fill_y <= Y_MAX; fill_y++) {
      VDP_setTileMapXY(BG_PLANE, VRAM_POS_TILE_FILL, fill_x, fill_y);
    }
  }

  // Place the cursor in its initial position
  VDP_setTileMapXY(CURSOR_PLANE, VRAM_POS_TILE_CURSOR, cursor_x, cursor_y);

  // Load different tiles on screen as buttons are pressed
  while(1)
  {
    // Wait for the VDP to finish drawing the frame so that screen
    // updates work as expected.
    VDP_waitVSync();

    // Handle cursor moves triggered by the gamepad input handler
    if (move_dir != NULL) {
      // Clear the cursor tile's current position
      VDP_setTileMapXY(CURSOR_PLANE, NULL, cursor_x, cursor_y);

      // Move the cursor position, restricting to viewable areas
      if (move_dir == DIR_RIGHT && cursor_x < X_MAX) {
        cursor_x++;
      } else if (move_dir == DIR_LEFT && cursor_x > X_MIN) {
        cursor_x--;
      } else if (move_dir == DIR_UP && cursor_y > Y_MIN) {
        cursor_y--;
      } else if (move_dir == DIR_DOWN && cursor_y < Y_MAX) {
        cursor_y++;
      }

      // Redraw the cursor in its new position
      VDP_setTileMapXY(CURSOR_PLANE, VRAM_POS_TILE_CURSOR, cursor_x, cursor_y);

      // Reset the move direction, allowing for more input
      move_dir = NULL;
    }
  }
}
