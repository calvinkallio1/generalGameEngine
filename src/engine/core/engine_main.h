#pragma once
/* Include this in the ONE file that defines main(). On Windows a windowed program
 * needs a WinMain; SDL provides it and calls your ordinary int main(void). Harmless
 * on macOS and Linux. Nothing else in a game needs to name SDL. */
#include <SDL3/SDL_main.h>
