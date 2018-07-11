#include <fstream>
#include <thread>
#include <SFML/Window.hpp>
#include <cmath>
#include <json.hpp>
#include "game.h"
#include "asset_registry.h"
#include "map_system.h"
#include "building_component.h"
#include "ui_terrain_info_window.h"

namespace Archipelago {

	// General game constants
	const std::string& gameName{ "Archipelago" };
	extern const std::string& loggerName{ gameName + "_logger" };
	const std::string& mapFileName{ "assets/maps/default_map.json" };
	const size_t stringReservationSize{ 100 };
	// Time constants
	const unsigned int gameMonthDurationNormal{ 30 };
	const unsigned int gameMonthDurationFast{ 10 };
	const unsigned int gameMonthDurationSuperFast{ 1 };
	// Camera keyboard control constants
	const int cameraMoveInterval{ 1 }; /// minimal interval between move steps in milliseconds
	const float cameraMoveStep{ 15 }; /// move step in pixels

}

using namespace Archipelago;
using namespace spdlog;
using namespace ECS;

Game::Game(): _isFullscreen(true), _windowWidth(800), _windowHeight(600), _curCameraZoom(1.0f) {}
Game::~Game() {}

void Game::init() {
	// Init logger
	_logger = basic_logger_mt(loggerName, "archipelago.log");
	_logger->set_level(level::trace);
	_logger->info("** {} starting **", gameName);

	// Load, parse and apply configuration settings
	std::fstream configFile;
	std::string configString;

	nlohmann::json configJSON;
	configFile.open("config.json");
	if (configFile.fail()) {
		_logger->error("Game::init failed. Error opening configuration file 'config.json'");
		return;
	}
	configFile >> configJSON;
	configFile.close();

	unsigned int logLevel;
	try {
		logLevel = configJSON.at("logging").at("level");
		if (logLevel > 6) {
			logLevel = 6;
		};
	}
	catch (std::out_of_range& e) {
		_logger->trace("No 'logging'.'level' option found in configuration file, default log level is 0 (trace). Error: {}", e.what());
	}
	_logger->trace("Logging level set to " + std::to_string(logLevel));
	_logger->set_level(static_cast<level::level_enum>(logLevel));

	try {
		_isFullscreen = configJSON.at("video").at("isFullscreen");
		_logger->trace("isFullscreen: {}", _isFullscreen);
		_windowWidth = configJSON.at("video").at("windowWidth");
		_logger->trace("windowWidth: {}", _windowWidth);
		_windowHeight = configJSON.at("video").at("windowHeight");
		_logger->trace("windowHeight: {}", _windowHeight);
		_enable_vsync = configJSON.at("video").at("enableVSync");
		_logger->trace("enableVSync: {}", _enable_vsync);
	}
	catch (const std::out_of_range& e) {
		_logger->error("Error parsing 'video' configuration object: {}", e.what());
		exit(-1);
	}

	// Determine some hardware facts
	_numThreads = std::thread::hardware_concurrency();
	_logger->info("Host has {} cores", _numThreads);

	// Init game variables
	_statusString.reserve(stringReservationSize);
	_cameraMoveIntervalCooldown = cameraMoveInterval;
	_mouseState = MouseState::Normal;

	// Init game subsystems
	_initRenderSystem();
	_assetRegistry = std::make_unique<Archipelago::AssetRegistry>();
	_assetRegistry->prepareWaresAtlas();
	_assetRegistry->prepareNaturalResourcesAtlas();
	_assetRegistry->prepareBuildingAtlas();
	_assetRegistry->loadTexture("mouse_cursor_normal", "assets/textures/mouse_cursor_normal.png");
	_assetRegistry->loadTexture("triangle_atention", "assets/textures/triangle_atention.png");
	_assetRegistry->loadTexture("dark_deep_space", "assets/textures/dark_deep_space.png");
	_setMouseCursorNormal();
	_world = World::createWorld();
	_world->registerSystem(new Archipelago::MapSystem(*this));
	_world->emit<LoadMapEvent>({ mapFileName });
	_world->emit<MoveCameraToMapCenterEvent>({ true });
	_initSettlementGoods();

	_ui = std::make_unique<Archipelago::Ui>(this);
	_ui->updateSettlementWares();

	// Game time variables
	_gameTime = 0;
	_currentGameMonthDuration = gameMonthDurationNormal;
	_ui->updateGameTimeString();
}

void Game::shutdown() {
	_ui.release(); // UI must be destroyed before world, because it need world to unsubscribe from its events
	_world->destroyWorld();
	_assetRegistry.release();
	_logger->info("** Archipelago finishing **");
	_logger->flush();
}

void Game::run() {
	sf::Event event;
	sf::Time frameTime;

	while (_window->isOpen()) {
		// Events & input processing
		while (_window->pollEvent(event)) {
			_ui->handleEvent(event);
			_processEvents(event);
		}
		_processInput(frameTime);
		
		// Turn over everything
		_update(frameTime);
		_draw();
		frameTime = _clock.restart();
	}
}

std::string Game::composeGameTimeString() {
	std::string timeString;
	unsigned int year = _gameTime / 12;
	unsigned int month = _gameTime - (year * 12) + 1;
	timeString = "Year ";
	timeString += std::to_string(year);
	timeString += ", Month ";
	timeString += std::to_string(month);
	switch (_currentGameMonthDuration) {
	case gameMonthDurationNormal:
		timeString += " (going normal)";
		break;
	case gameMonthDurationFast:
		timeString += " (going fast)";
		break;
	case gameMonthDurationSuperFast:
		timeString += " (going superfast)";
		break;
	}
	return timeString;
}

void Game::onUISelectBuilding(BuildingTypeId buildingID) {
	_logger->trace("Game::onUISelectBuilding called with id {}", std::underlying_type<WaresTypeId>::type(buildingID));
	BuildingSpecification bs = _assetRegistry->getBuildingSpecification(buildingID);
	_mouseState = MouseState::BuildingPlacement;
	_selectedForBuilding = buildingID;
	_mouseSprite.setTexture(*bs.icon, true);
}

void Game::_initRenderSystem() {
	// Init render window
	int windowStyle = sf::Style::Default;
	sf::VideoMode videoMode(static_cast<unsigned int>(_windowWidth), static_cast<unsigned int>(_windowHeight));
	if (_isFullscreen) {
		videoMode = videoMode.getDesktopMode();
		windowStyle = sf::Style::Fullscreen;
	};
	_window = std::make_unique<sf::RenderWindow>(videoMode, gameName, windowStyle);
	_window->setVerticalSyncEnabled(_enable_vsync);
	_window->setMouseCursorVisible(false);
	_window->setKeyRepeatEnabled(false);
	_isMovingCamera = false;
}

void Game::_initSettlementGoods() {
	for (WaresTypeId gti = WaresTypeId::_First; gti <= WaresTypeId::_Last; gti = static_cast<WaresTypeId>(std::underlying_type<WaresTypeId>::type(gti) + 1)) {
		WaresStack stack;
		stack.type = gti;
		stack.amount = 0;
		_settlementWares.push_back(std::move(stack));
	}
}

void Game::_processEvents(sf::Event event) {
	switch (event.type) {
	case sf::Event::Resized: {
		_ui->resizeUi(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
	}
	break;
	case sf::Event::KeyPressed: {
		switch (event.key.code) {
		case sf::Keyboard::Escape: {
			_window->close();
		}
		break;
		case sf::Keyboard::Add: {
			switch (_currentGameMonthDuration) {
			case gameMonthDurationNormal:
				_currentGameMonthDuration = gameMonthDurationFast;
				break;
			case gameMonthDurationFast:
				_currentGameMonthDuration = gameMonthDurationSuperFast;
				break;
			}
			_ui->updateGameTimeString();
		}
		break;
		case sf::Keyboard::Subtract: {
			switch (_currentGameMonthDuration) {
			case gameMonthDurationSuperFast:
				_currentGameMonthDuration = gameMonthDurationFast;
				break;
			case gameMonthDurationFast:
				_currentGameMonthDuration = gameMonthDurationNormal;
				break;
			}
			_ui->updateGameTimeString();
		}
		break;
		case sf::Keyboard::Space: {
			_world->emit<ShowNaturalResourcesEvent>({ true });
		}
		break;
		}
	}
	break;
	case sf::Event::KeyReleased: {
		switch (event.key.code) {
		case sf::Keyboard::Space:
			_world->emit<ShowNaturalResourcesEvent>({ false });
		break;
		}
	}
	break;
	case sf::Event::MouseMoved: {
		_processMouseMovement();
		_hideTerrainInfoWindow();
	}
	break;
	case sf::Event::MouseWheelMoved: {
		if (event.mouseWheel.delta < 0) {
			_zoomCamera(1.5f);
		}
		else
		{
			_zoomCamera(0.5f);
			
		}
	}
	break;
	case sf::Event::Closed: {
		_window->close();
	}
	break;
	case sf::Event::MouseButtonPressed: {
		if (event.mouseButton.button == sf::Mouse::Right) {
			_isMovingCamera = true;
			_prevMouseCoords = sf::Mouse::getPosition(*_window);
		}
	}
	break;
	case sf::Event::MouseButtonReleased: {
		if (event.mouseButton.button == sf::Mouse::Left) {
			if (_mouseState == MouseState::BuildingPlacement) {
				_placeBuilding();
			};
			if (_mouseState == MouseState::Normal) {
				_showTerrainInfoWindow();
			}			
		}
		if (event.mouseButton.button == sf::Mouse::Right) {
			_isMovingCamera = false;
			_setMouseCursorNormal();
		};
	}
	break;
	}
}

void Game::_processInput(const sf::Time& frameTime) {
	// Camera move stepping
	bool canMoveCamera = false;
	if (_cameraMoveIntervalCooldown > 0) {
		_cameraMoveIntervalCooldown -= frameTime.asMilliseconds();
	}
	else {
		_cameraMoveIntervalCooldown = cameraMoveInterval;
		canMoveCamera = true;
	}
	if (canMoveCamera) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			_world->emit<MoveCameraEvent>({ -cameraMoveStep, 0.0f });
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			_world->emit<MoveCameraEvent>({ cameraMoveStep, 0.0f });
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			_world->emit<MoveCameraEvent>({ 0.0f, -cameraMoveStep });
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			_world->emit<MoveCameraEvent>({ 0.0f, cameraMoveStep });
		}
		_processMouseMovement();
	}
}

void Game::_update(const sf::Time& frameTime) {
	// Update status string
	_fps = static_cast<int>(1.0f / frameTime.asSeconds());
	_statusString = " FPS: ";
	_statusString += std::to_string(_fps);

	// Update game time
	_accumulatedTime += frameTime;
	if (_accumulatedTime.asSeconds() >= _currentGameMonthDuration) {
		_gameTime++;
		_accumulatedTime = sf::Time::Zero;
		_updateSettlement();
		_ui->updateGameTimeString();
	}

	// Update world
	_world->tick(0);

	// Update UI
	_ui->update(frameTime.asSeconds());
}

void Game::_draw() {
	_window->clear();
	// Render background
	_drawBackgroundImage();
	// Render map
	_world->emit<RenderMapEvent>({ true });
	// Render UI
	_ui->render();
	// Render mouse cursor
	if (_mouseState == MouseState::BuildingPlacement) {
		auto ent = _world->getById(_getEntityIDUnderCursor());
		if (ent &&
			_settlementHasWaresForBuilding(_assetRegistry->getBuildingSpecification(_selectedForBuilding)) &&
			!_settlementExceededAllowedBuildingAmount(_assetRegistry->getBuildingSpecification(_selectedForBuilding)) &&
			_requiredNatresPresentOnTile(ent, _selectedForBuilding) &&
			ent->get<BuildingComponent>() == ComponentHandle<BuildingComponent>(nullptr)) {
			_mouseSprite.setColor(sf::Color(255, 255, 255, 127));
		}
		else {
			_mouseSprite.setColor(sf::Color(255, 0, 0, 127));
		}
	}
	_window->draw(_mouseSprite);
	// Show everything on screen
	_window->display();
}

void Game::_drawBackgroundImage() {
	constexpr float PARALLAX_MULTIPLIER{ 50.f };
	constexpr float BKG_PARALLAX_COMPENSATION_SCALE{ 1.1f };

	sf::Texture* bkgTex{ _assetRegistry->getTexture("dark_deep_space") };
	sf::Sprite bkgSpr{ *bkgTex };
	sf::View v = getRenderWindow().getView();

	sf::Vector2f viewCenter{ v.getCenter() };
	sf::Vector2f viewSize{ v.getSize() };
	sf::Vector2f texSize{ bkgTex->getSize() };
	sf::Vector2f sprBaseScale{ viewSize.x / texSize.x, viewSize.y / texSize.y };
	sf::Vector2f parallaxOffset{ viewCenter / PARALLAX_MULTIPLIER };

	bkgSpr.setScale(sprBaseScale * BKG_PARALLAX_COMPENSATION_SCALE);
	bkgSpr.setOrigin(texSize / 2.0f); // Set origin to center of texture
	bkgSpr.setPosition(viewCenter - parallaxOffset);
	getRenderWindow().draw(bkgSpr);
}

void Game::_setMouseCursorNormal() {
	_mouseSprite.setTexture(*_assetRegistry->getTexture("mouse_cursor_normal"), true);
	_mouseSprite.setPosition(_window->mapPixelToCoords(sf::Mouse::getPosition(*_window)));
	_mouseSprite.setColor(sf::Color::White);
	_mouseState = MouseState::Normal;
}

void Game::_processMouseMovement() {
	int cursorOffsetX{ 0 };
	int cursorOffsetY{ 0 };
	if (_mouseState == MouseState::BuildingPlacement) {
		cursorOffsetX = -(int)(_mouseSprite.getTexture()->getSize().x * 0.5f);
		cursorOffsetY = -(int)(_mouseSprite.getTexture()->getSize().y * 0.5f);
	}
	_mouseSprite.setPosition(_window->mapPixelToCoords(
		sf::Vector2i(
		(int)(sf::Mouse::getPosition(*_window).x + cursorOffsetX),
			(int)(sf::Mouse::getPosition(*_window).y + cursorOffsetY)
		)
	));
	_world->emit<MouseMovedEvent>({ true });
	if (_isMovingCamera) {
		_world->emit<MoveCameraEvent>({
			static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).x),
			static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).y)
		});
		_prevMouseCoords = sf::Mouse::getPosition(*_window);
	}
}

void Game::_zoomCamera(float zoomFactor) {
	if (_curCameraZoom * zoomFactor > maxCameraZoom || _curCameraZoom * zoomFactor < minCameraZoom) {
		return;
	}
	sf::View v = _window->getView();
	v.zoom(zoomFactor);
	_window->setView(v);
	_curCameraZoom *= zoomFactor;
	_mouseSprite.scale(sf::Vector2f(zoomFactor, zoomFactor));
}

void Game::_updateSettlement() {
	_world->each<BuildingComponent>([&](Entity* ent, ComponentHandle<BuildingComponent> bc) {
		const BuildingSpecification& bs{ *bc->spec };
		for (WaresStack waresStack : bs.waresProduced) {
			WaresTypeId type{ waresStack.type };
			int amount{ waresStack.amount };
			for (auto& settWare : _settlementWares) {
				if (settWare.type == type) {
					settWare.amount += amount;
				}
			}
		}
		_ui->updateSettlementWares();
	});
}

bool Game::_settlementHasWaresForBuilding(const BuildingSpecification& bs) {
	for (auto& settWare : _settlementWares) {
		for (auto& requiredWare : bs.waresRequired) {
			if (settWare.type == requiredWare.type) {
				if (settWare.amount < requiredWare.amount) return false;
			}
		}
	}
	return true;
}

bool Game::settlementHasWareForBuilding(const BuildingSpecification& bs, WaresTypeId ware) {
	int amountNeeded{ 0 };
	for (auto& requiredWare : bs.waresRequired) {
		if (requiredWare.type == ware) {
			amountNeeded = requiredWare.amount;
		}
	}
	for (auto& settWare : _settlementWares) {
		if (settWare.type == ware && settWare.amount >= amountNeeded)
			return true;
	}
	return false;
}

bool Game::_settlementExceededAllowedBuildingAmount(const BuildingSpecification& bs) {
	if (bs.maxAllowedOnMap == 0) {
		return false;
	}
	unsigned int buildingsCount{ 0 };
	_world->each<BuildingComponent>([&](Entity* ent, ComponentHandle<BuildingComponent> bc) {
		const BuildingSpecification& this_bs{ *bc->spec };
		if (this_bs.id == bs.id) {
			++buildingsCount;
		}
	});
	if (buildingsCount >= bs.maxAllowedOnMap) {
		return true;
	}
	return false;
}

bool Game::_requiredNatresPresentOnTile(ECS::Entity* ent, BuildingTypeId buildingID) {
	auto natres = ent->get<NaturalResourceComponent>();
	uint32_t resourseSet = 0;
	if (natres) {
		resourseSet = natres->resourceSet;
	}
	if (resourseSet == 0) return false;
	NaturalResourceTypeId natresRequired = _assetRegistry->getBuildingSpecification(buildingID).natresRequired;
	if (natresRequired == NaturalResourceTypeId::Unknown) return true;
	if (NaturalResourceTypeId(resourseSet & 0x000000FF) != natresRequired) return false;
	return true;
}

void Game::_placeBuilding() {
	if (_mouseState != MouseState::BuildingPlacement) return;
	const BuildingSpecification& bs = _assetRegistry->getBuildingSpecification(_selectedForBuilding);
	if (!_settlementHasWaresForBuilding(bs) || _settlementExceededAllowedBuildingAmount(bs)) return;
	if (_getEntityIDUnderCursor() == 0) return;
	auto ent = _world->getById(_getEntityIDUnderCursor());
	if (!ent) return;
	if (!_requiredNatresPresentOnTile(ent, _selectedForBuilding)) return;
	ComponentHandle<TileComponent> tile = ent->get<TileComponent>();
	if (tile == ComponentHandle<TileComponent>(nullptr)) {
		_logger->warn("[Game::_placeBuilding] Tile component not found on world entity!");
		return;
	}
	ComponentHandle<BuildingComponent> building = ent->get<BuildingComponent>();
	if (building == ComponentHandle<BuildingComponent>(nullptr)) {
		ent->assign<BuildingComponent>(&bs);
		building = ent->get<BuildingComponent>();
	}
	else { // Tile is occupied with another building
		return;
	}
	building->sprite.setTexture(*(bs.icon), true);
	sf::Vector2f pos = tile->sprite.getPosition();
	pos.y += (float)tile->rising - (float)bs.tileRising;
	building->sprite.setPosition(pos);
	for (auto& settWare : _settlementWares) {
		for (auto& requiredWare : bs.waresRequired) {
			if (settWare.type == requiredWare.type) {
				settWare.amount -= requiredWare.amount;
			}
		}
		for (auto& providedInstantWare : bs.providedInstantWares) {
			if (settWare.type == providedInstantWare.type) {
				settWare.amount += providedInstantWare.amount;
			}
		}
	}
	_ui->updateSettlementWares();
	_setMouseCursorNormal();
}

size_t Game::_getEntityIDUnderCursor() {
	size_t entityID;
	_world->emit<RequestHighlightedEntityEvent>({ entityID });
	return entityID;
}

void Game::_showTerrainInfoWindow() {
	size_t entId = _getEntityIDUnderCursor();
	if (!entId) return;

	TerrainInfoWindowDataUpdateEvent tiwData;
	tiwData.show = true;
	tiwData.position = sf::Vector2f(sf::Mouse::getPosition(*_window)) + sf::Vector2f((float)_mouseSprite.getTextureRect().width, 0.0f);
	if (_world->getById(entId)->has<BuildingComponent>()) {
		auto building = _world->getById(entId)->get<BuildingComponent>().get();
		tiwData.tileType = TileType::BUILDING;
		tiwData.tileSprite = &building.sprite;
		tiwData.name = building.spec->name;
		tiwData.buildingDescription = building.spec->description;
		tiwData.production = &building.spec->waresProduced;
	}
	else {
		auto tile = _world->getById(entId)->get<TileComponent>().get();
		auto res = _world->getById(entId)->get<NaturalResourceComponent>().get();
		tiwData.tileType = TileType::TERRAIN;
		tiwData.tileSprite = &(tile.sprite);
		tiwData.name = tile.name;
		tiwData.resourceSet = res.resourceSet;
	}
	_world->emit<TerrainInfoWindowDataUpdateEvent>(tiwData);
}

void Game::_hideTerrainInfoWindow() {
	_ui->hideTerrainInfoWindow();
}
