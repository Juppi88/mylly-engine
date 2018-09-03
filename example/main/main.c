#include "core/mylly.h"

static void main_loop(void)
{

}

int main(int argc, char **argv)
{
	// Initiliaze the engine and enter the main loop.
	if (mylly_initialize(argc, argv)) {
		mylly_main_loop(main_loop);
	}

	return 0;
}
