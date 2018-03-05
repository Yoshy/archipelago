#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <ECS.h>
#include "asset_registry.h"
#include "ui.h"

namespace Archipelago {

	extern const std::string& loggerName;

	enum class MouseState { Normal = 0, BuildingPlacement = 1 };
	const float maxCameraZoom{ 3.0f };
	const float minCameraZoom{ 0.2f };

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
		ECS::World* getWorld() const { return _world; };
		const float getRenderWindowWidth() const { return _windowWidth; };
		const float getRenderWindowHeight() const { return _windowHeight; };
		std::string composeGameTimeString(void);
		const std::string& getStatusString() const { return _statusString; };
		const size_t getSettlementWaresNumber() const { return _settlementWares.size(); };
		const sf::Image getWareIcon(unsigned int idx) const { return _assetRegistry->getWaresSpecification(_settlementWares[idx].type).icon->copyToImage(); };
		const int getWareAmount(unsigned int idx) const { return _settlementWares[idx].amount; };
		void onUISelectBuilding(BuildingTypeId buildingID);
	private:
		void _initRenderSystem();
		void _initSettlementGoods();
		void _processEvents(sf::Event event);
		void _processInput(const sf::Time& frameTime);
		void _update(const sf::Time& frameTime);
		void _draw();
		void _setMouseCursorNormal();
		void _processMouseMovement();
		void _zoomCamera(float zoomFactor);
		bool _requiredNatresPresentOnTile(ECS::Entity* ent, BuildingTypeId buildingID);
		bool _settlementHasWaresForBuilding(const BuildingSpecification& bs);
		void _placeBuilding();
		size_t _getEntityIDUnderCursor();
		void _showTerrainInfoWindow();
		void _hideTerrainInfoWindow();

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
		MouseState _mouseState;
		sf::Sprite _mouseSprite;

		// game mechanic stuff
		unsigned int _gameTime; // Months since game start
		unsigned int _currentGameMonthDuration; // Game month duration in realtime seconds
		std::vector<WaresStack> _settlementWares; // Current stock of settlement wares
		BuildingTypeId _selectedForBuilding;
		
		// auxilary vars
		std::string _statusString;
		sf::Time _accumulatedTime{ sf::Time::Zero };
		int _cameraMoveIntervalCooldown; // milliseconds
		unsigned int _numThreads;
		unsigned int _fps;
		bool _isMovingCamera;
		sf::Vector2i _prevMouseCoords;
		float _curCameraZoom;
		sf::Clock _clock;
	};

}
