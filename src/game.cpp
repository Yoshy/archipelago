#include <fstream>
#include <thread>
#include <SFML/Window.hpp>
#include <cmath>
#include <json.hpp>
#include "game.h"
#include "asset_registry.h"

namespace Archipelago {

	// General game constants
	const std::string& gameName{ "Archipelago" };
	extern const std::string& loggerName{ gameName + "_logger" };
	const std::string& defaultMapName{ "default_map" };
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

Game::Game(): _isFullscreen(true), _windowWidth(800), _windowHeight(600) {

}

Game::~Game() {

}

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

	// Init game subsystems
	_mousePositionString.reserve(stringReservationSize);
	_statusString.reserve(stringReservationSize);
	_cameraMoveIntervalCooldown = cameraMoveInterval;
	_curMapName = defaultMapName;
	_loadAssets();
	_initSettlementGoods();
	_initGraphics();
	_gameTime = 0;
	_currentGameMonthDuration = gameMonthDurationNormal;
}

void Game::shutdown() {
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

void Game::_initGraphics() {
	// Init render window
	int windowStyle = sf::Style::Default;
	sf::VideoMode videoMode(static_cast<unsigned int>(_windowWidth), static_cast<unsigned int>(_windowHeight));
	if (_isFullscreen) {
		videoMode = videoMode.getDesktopMode();
		windowStyle = sf::Style::Fullscreen;
	};
	_window = std::make_unique<sf::RenderWindow>(videoMode, gameName, windowStyle);
	_window->setVerticalSyncEnabled(_enable_vsync);

	// Init viewport
	sf::View v = _window->getView();
	v.setCenter(_assetRegistry->getMap(_curMapName).getCenter());
	_window->setView(v);
	_window->setMouseCursorVisible(true);
	_isMovingCamera = false;

	// Init UI
	_ui = std::make_unique<Archipelago::Ui>(*this);
}

void Game::_loadAssets() {
	if (!_assetRegistry) {
		_assetRegistry = std::make_unique<Archipelago::AssetRegistry>();
	}
	_assetRegistry->prepareWaresAtlas();
	// map must be loaded last, because it needs other assets, such as wares
	_assetRegistry->loadMap(_curMapName, "assets/maps/" + _curMapName + ".json");
	_prevTile = nullptr;
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

void Game::_initSettlementGoods() {
	for (WaresTypeId gti = WaresTypeId::_Begin; gti != WaresTypeId::_End; gti = static_cast<WaresTypeId>(std::underlying_type<WaresTypeId>::type(gti) + 1)) {
		WaresStack stack;
		stack.type = gti;
		stack.amount = 0;
		_settlementWares.push_back(std::move(stack));
	}
}

void Game::_processEvents(sf::Event event) {
	switch (event.type) {
	case sf::Event::Resized:
		_ui->resizeUi(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
		break;
	case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::Escape:
			_window->close();
			break;
		case sf::Keyboard::Add:
			switch (_currentGameMonthDuration) {
			case gameMonthDurationNormal:
				_currentGameMonthDuration = gameMonthDurationFast;
				break;
			case gameMonthDurationFast:
				_currentGameMonthDuration = gameMonthDurationSuperFast;
				break;
			}
			break;
		case sf::Keyboard::Subtract:
			switch (_currentGameMonthDuration) {
			case gameMonthDurationSuperFast:
				_currentGameMonthDuration = gameMonthDurationFast;
				break;
			case gameMonthDurationFast:
				_currentGameMonthDuration = gameMonthDurationNormal;
				break;
			}
			break;
		}
	case sf::Event::MouseMoved:
		if (_isMovingCamera) {
			_moveCamera(static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).x),
				static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).y));
			_prevMouseCoords = sf::Mouse::getPosition(*_window);
		}
		_getMousePositionString(_mousePositionString);
		break;
	case sf::Event::MouseWheelMoved:
		if (event.mouseWheel.delta < 0) {
			_ui->zoomCamera(1.5f);
		}
		else
		{
			_ui->zoomCamera(0.5f);
		}
		break;
	case sf::Event::Closed:
		_window->close();
		break;
	case sf::Event::MouseButtonPressed:
		if (event.mouseButton.button == sf::Mouse::Right) {
			_isMovingCamera = true;
			_prevMouseCoords = sf::Mouse::getPosition(*_window);
		}
		break;
	case sf::Event::MouseButtonReleased:
		if (event.mouseButton.button == sf::Mouse::Right) {
			_isMovingCamera = false;
		};
		if (event.mouseButton.button == sf::Mouse::Left) {
			//_showTerrainInfo();
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
	// Keyboard state processing
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
		_assetRegistry->getMap(_curMapName).setWaresVisibility(true);
	}
	else {
		_assetRegistry->getMap(_curMapName).setWaresVisibility(false);
	}
	if (canMoveCamera) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			_moveCamera(-cameraMoveStep, 0.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			_moveCamera(cameraMoveStep, 0.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			_moveCamera(0.0f, -cameraMoveStep);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			_moveCamera(0.0f, cameraMoveStep);
		}
	}
}

void Game::_update(const sf::Time& frameTime) {
	// Update status string
	_fps = static_cast<int>(1.0f / frameTime.asSeconds());
	_statusString = _mousePositionString;
	_statusString += " FPS: ";
	_statusString += std::to_string(_fps);

	// Update game time
	_accumulatedTime += frameTime;
	if (_accumulatedTime.asSeconds() >= _currentGameMonthDuration) {
		_gameTime++;
		_accumulatedTime = sf::Time::Zero;
	}

	// Update UI
	_ui->update(frameTime.asSeconds());
}

void Game::_draw() {
	_window->clear();

	// Render world
	sf::Vector2f screenCoords = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
	sf::Vector2f mapCoords = _assetRegistry->getMap(_curMapName).screenToMapCoords(screenCoords);
	Tile* tile = _assetRegistry->getMap(_curMapName).getTileAt(static_cast<int>(floor(mapCoords.x)), static_cast<int>(floor(mapCoords.y)));
	if (tile) {
		if (_prevTile) {
			_prevTile->getSprite().setColor(sf::Color::White);
		}
		tile->getSprite().setColor(sf::Color(255, 255, 255, 127));
		_prevTile = tile;
	}
	_assetRegistry->getMap(_curMapName).draw(*_window);

	// Display everything
	_ui->display();
	_window->display();
}

void Game::_moveCamera(float offsetX, float offsetY) {
	sf::View v = _window->getView();
	sf::Vector2f viewCenter = v.getCenter();
	viewCenter.x = viewCenter.x + offsetX;
	viewCenter.y = viewCenter.y + offsetY;
	sf::Vector2f mapCoords = _assetRegistry->getMap(_curMapName).screenToMapCoords(viewCenter);
	if (mapCoords.x < 0 || mapCoords.y < 0 || mapCoords.x > _assetRegistry->getMap(_curMapName).getMapWidth() || mapCoords.y > _assetRegistry->getMap(_curMapName).getMapHeight()) {
		return;
	}
	v.move(offsetX, offsetY);
	_window->setView(v);
}

void Game::_getMousePositionString(std::string& str) {
	sf::Vector2f screenCoords = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
	sf::Vector2f mapCoords = _assetRegistry->getMap(_curMapName).screenToMapCoords(screenCoords);
	str = "Screen X: " + std::to_string(screenCoords.x) + " Y: " + std::to_string(screenCoords.y) + "; ";
	str += "World X:" + std::to_string(mapCoords.x) + " Y: " + std::to_string(mapCoords.y) + "; ";
}
