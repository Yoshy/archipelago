/*

�������� ����� ����, ����� �����.
���������, ������������� � �������������� �������� ����������

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

// Duration of game month in real time seconds
#define GAME_MONTH_DURATION_NORMAL 30
#define GAME_MONTH_DURATION_FAST 10
#define GAME_MONTH_DURATION_SUPERFAST 1

#define UI_STATUS_BAR_HEIGHT 35.0f
#define UI_TSB_TIME_LABEL_ID "tsb_time_label"
#define UI_BOTTOM_STATUS_BAR_LABEL_ID "bsb_label"
#define UI_TSB_GOODS_LABEL "tsb_goods_label"

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
		// game mechanic stuff
		unsigned int _gameTime; // Months since game start
		unsigned int _currentGameMonthDuration;
		std::string _getCurrentGameTimeString(void);
		std::vector<GoodsStack> _settlementGoods;
		void _initSettlementGoods();
		// UI stuff
		sfg::Window::Ptr _uiTopStatusBar;
		sfg::Window::Ptr _uiBottomStatusBar;
		sfg::Window::Ptr _uiTerrainInfoWindow;
		void _resizeUi(unsigned int width, unsigned int height);
		void _showTerrainInfo();
		void _onTerrainInfoWindowMouseEnter();
		void _onTerrainInfoWindowMouseLeave();
		// game internal stuff
		Tile* _prevTile;
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
		// game class posessions
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
