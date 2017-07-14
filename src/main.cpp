#include <windows.h>
#include "game.h"

//int main() {
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT) {
	Apchipelago::Game game;

	try {
		game.init();
	}
	catch (...) {
		return 0;
	}
	game.run();
	return 0;
}