#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <ECS.h>
#include "asset_registry.h"
#include "ui.h"

namespace Archipelago {

	extern const std::string& loggerName;

	/** Main game class, entry point.
	* Loads, configures and initialises all the components
	*/
	class Game {
	public:
		Game();
		Game(const Game&) = delete;
		~Game();
		void init();
		void run();
		void shutdown();

		sf::RenderWindow& getRenderWindow() const { return *_window; };
		Archipelago::AssetRegistry& getAssetRegistry() const { return *_assetRegistry; }
		const float getRenderWindowWidth() const { return _windowWidth; };
		const float getRenderWindowHeight() const { return _windowHeight; };
		std::string composeGameTimeString(void);
		const std::string& getStatusString() const { return _statusString; };
		const size_t getSettlementWaresNumber() const { return _settlementWares.size(); };
		const sf::Image getWareIcon(unsigned int idx) const { return _assetRegistry->getWaresSpecification(_settlementWares[idx].type).icon->copyToImage(); };
		const unsigned int getWareAmount(unsigned int idx) const { return _settlementWares[idx].amount; };
	private:
		void _initGraphics();
		void _initSettlementGoods();
		void _processEvents(sf::Event event);
		void _processInput(const sf::Time& frameTime);
		void _update(const sf::Time& frameTime);
		void _draw();
		void _moveCamera(float offsetX, float offsetY);
		void _getMousePositionString(std::string& str);

		// game posessions
		std::shared_ptr<spdlog::logger> _logger;
		std::unique_ptr<Archipelago::AssetRegistry> _assetRegistry;
		std::unique_ptr<sf::RenderWindow> _window;
		std::unique_ptr<Archipelago::Ui> _ui;
		ECS::World* _world;

		// game options (see config.json)
		bool _isFullscreen;
		bool _enable_vsync;
		float _windowWidth, _windowHeight;

		// game mechanic stuff
		std::string _curMapName;
		unsigned int _gameTime; // Months since game start
		unsigned int _currentGameMonthDuration; // Game month duration in realtime seconds
		std::vector<WaresStack> _settlementWares; // Current stock of settlement wares
		
		// auxilary vars
		//Tile* _prevTile;
		std::string _mousePositionString;
		std::string _statusString;
		sf::Time _accumulatedTime{ sf::Time::Zero };
		int _cameraMoveIntervalCooldown; // milliseconds
		unsigned int _numThreads;
		unsigned int _fps;
		bool _isMovingCamera;
		sf::Vector2i _prevMouseCoords;
		sf::Clock _clock;
	};

}
