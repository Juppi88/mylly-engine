#include "game.h"
#include <mylly/core/mylly.h>

// Game handler instance.
static Game game;

// This method is called on every frame before rendering the scene.
static void MainLoop(void)
{
	game.Update();
}

int main(int argc, char **argv)
{
	// Initialize the engine and enter the main loop.
	if (mylly_initialize(argc, argv)) {

		game.SetupGame();
		mylly_main_loop(MainLoop);
	}

	game.Shutdown();

	return 0;
}
