/*

Основной класс игры, точка входа.
Загружает, конфигурирует и инициализирует основные компоненты

*/

#ifndef GAME_H
#define GAME_H

#include <spdlog\spdlog.h>
#include <memory>
#include <SFML/Graphics.hpp>

extern const char* GAME_NAME;

namespace Apchipelago {

	class Game {
	public:
		Game();
		Game(const Game&) = delete;
		~Game();
		void init();
		void run();
		void shutdown();
	private:
		// game internal stuff
		unsigned int _numThreads;
		double _fps;
		sf::Clock _clock;
		void _initGraphics();
		// game posessions
		std::shared_ptr<spdlog::logger> _logger;
		std::unique_ptr<sf::RenderWindow> _window;
		// game options
		bool _isFullscreen;
		int _windowWidth, _windowHeight;
	};

}

#endif // GAME_H
