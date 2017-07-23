#include <windows.h>
#include "game.h"

//int main() {
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT) {
	Archipelago::Game game;

	game.init();
	game.run();
	game.shutdown();
	return 0;
}