/*

Основной класс игры, точка входа.
Загружает, конфигурирует и инициализирует основные компоненты

*/

#ifndef GAME_H
#define GAME_H

#include <spdlog/spdlog.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include "map.h"

extern const char* GAME_NAME;

namespace Archipelago {

	class AssetRegistry;

	class Game {
	public:
		Game();
		Game(const Game&) = delete;
		~Game();
		void init();
		void run();
		void shutdown();
	private:
		sf::Font _font;
		sf::Text _mouseCoordsString;
		Tile* _prevTile;
		// game internal stuff
		unsigned int _numThreads;
		double _fps;
		float _curCameraZoom;
		bool _isMovingCamera;
		sf::Vector2i _prevMouseCoords;
		sf::Clock _clock;
		void _initGraphics();
		void _loadAssets();
		void _draw();
		void _moveCamera(float offsetX, float offsetY);
		void _zoomCamera(float zoomFactor);
		void _updateMousePositionString();
		// game posessions
		std::shared_ptr<spdlog::logger> _logger;
		std::unique_ptr<sf::RenderWindow> _window;
		std::unique_ptr<Archipelago::AssetRegistry> _assetRegistry;
		// game options
		bool _isFullscreen;
		int _windowWidth, _windowHeight;
	};

}

#endif // GAME_H
