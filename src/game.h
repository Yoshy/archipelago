/*

Основной класс игры, точка входа.
Загружает, конфигурирует и инициализирует основные компоненты

*/

#ifndef GAME_H
#define GAME_H

#include <spdlog/spdlog.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include "map.h"

extern const char* GAME_NAME;

#define STRING_RESERVATION_SIZE 100
#define UI_MAIN_WINDOW_WIDTH 100.0f

#define UI_TERRAIN_INFO_WINDOW_WIDTH 200.0f
#define UI_TERRAIN_INFO_WINDOW_HEIGHT 200.0f

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
		Tile* _prevTile;
		// game internal stuff
		unsigned int _numThreads;
		unsigned int _fps;
		float _curCameraZoom;
		bool _isMovingCamera;
		sf::Vector2i _prevMouseCoords;
		sf::Clock _clock;
		void _initGraphics();
		void _loadAssets();
		void _draw();
		void _moveCamera(float offsetX, float offsetY);
		void _zoomCamera(float zoomFactor);
		void _getMousePositionString(std::string& str);
		// UI stuff
		sfg::Window::Ptr _uiMainWindow;
		sfg::Label::Ptr _uiStatusBar;
		sfg::Window::Ptr _uiTerrainInfoWindow;
		void _showTerrainInfo();
		void _onTerrainInfoWindowMouseEnter();
		void _onTerrainInfoWindowMouseLeave();
		// game posessions
		std::shared_ptr<spdlog::logger> _logger;
		std::unique_ptr<Archipelago::AssetRegistry> _assetRegistry;
		std::unique_ptr<sf::RenderWindow> _window;
		std::unique_ptr<sfg::SFGUI> _sfgui;
		std::unique_ptr<sfg::Desktop> _uiDesktop;
		// game options (see config.json)
		bool _isFullscreen;
		bool _enable_vsync;
		int _windowWidth, _windowHeight;
	};

}

#endif // GAME_H
