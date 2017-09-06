#include "globals.h"
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <thread>
#include "game.h"
#include "asset_registry.h"
#include <SFML/Window.hpp>
#include <cmath>

using namespace Archipelago;
using namespace std;
using namespace spdlog;
using namespace rapidjson;

const char* GAME_NAME = "Archipelago";

Game::Game(): _isFullscreen(true), _windowWidth(800), _windowHeight(600) {

}

Game::~Game() {

}

void Game::init() {
	// Init logger
	_logger = basic_logger_mt(LOGGER_NAME, "archipelago.log");
	_logger->set_level(level::trace);
	_logger->info("** {} starting **", GAME_NAME);

	// Load, parse and apply configuration settings
	fstream configFile;
	string configString;
	configFile.open("config.json");
	string s;
	while (configFile >> s) {
		configString += s;
	}
	configFile.close();
	Document configDOM;
	ParseResult pr = configDOM.Parse<kParseCommentsFlag>(configString.c_str());
	if (!pr) {
		_logger->error("Error while parsing configuration file: '{}', offset: {}", GetParseError_En(pr.Code()), pr.Offset());
	}
	if (configDOM.HasMember("logging") && configDOM["logging"].IsObject()) {
		const Value& loggingObject = configDOM["logging"];
		Value::ConstMemberIterator iter = loggingObject.FindMember("level");
		if (iter != loggingObject.MemberEnd() && iter->value.IsUint()) {
			unsigned int logLevel = iter->value.GetUint();
			if (logLevel > 6) {
				logLevel = 6;
			}
			_logger->trace("Logging level set to " + to_string(logLevel));
			_logger->set_level(static_cast<level::level_enum>(logLevel));
		}
	}
	else {
		_logger->trace("No logLevel found in configuration file, default log level is 0 (trace)");
	}

	if (configDOM.HasMember("video") && configDOM["video"].IsObject()) {
		const Value& videoSettings = configDOM["video"];
		Value::ConstMemberIterator iter;
		iter= videoSettings.FindMember("isFullscreen");
		if (iter != videoSettings.MemberEnd() && iter->value.IsBool()) {
			_isFullscreen = iter->value.GetBool();
			_logger->trace("fullscreen: {}", _isFullscreen);
		}
		iter = videoSettings.FindMember("windowWidth");
		if (iter != videoSettings.MemberEnd() && iter->value.IsUint()) {
			_windowWidth = iter->value.GetUint();
			_logger->trace("windowWidth: {}", _windowWidth);
		}
		iter = videoSettings.FindMember("windowHeight");
		if (iter != videoSettings.MemberEnd() && iter->value.IsUint()) {
			_windowHeight = iter->value.GetUint();
			_logger->trace("windowHeight: {}", _windowHeight);
		}
	}

	// Determine some hardware facts
	_numThreads = std::thread::hardware_concurrency();
	_logger->info("Host has {} cores", _numThreads);

	// Init game subsystems
	_loadAssets();
	_initGraphics();
}

void Game::shutdown() {
	_logger->info("** Archipelago finishing **");
	_logger->flush();
}

void Game::run() {
	sf::Time time;
	unsigned int skippedFrames = 0;

	while (_window->isOpen()) {
		_clock.restart();

		// Events processing
		sf::Event event;
		while (_window->pollEvent(event)) {
			switch (event.type) {
			case sf::Event::KeyPressed:
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					_window->close();
					break;
				case sf::Keyboard::Left:
					_moveCamera(-static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileWidth()), 0);
					break;
				case sf::Keyboard::Right:
					_moveCamera(static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileWidth()), 0);
					break;
				case sf::Keyboard::Up:
					_moveCamera(0, -static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileHeight()));
					break;
				case sf::Keyboard::Down:
					_moveCamera(0, static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileHeight()));
					break;
				}
			case sf::Event::MouseMoved:
				if (_isMovingCamera) {
					_moveCamera(static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).x),
						        static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).y));
					_prevMouseCoords = sf::Mouse::getPosition(*_window);
				}
				_updateMousePositionString();
				break;
			case sf::Event::MouseWheelMoved:
				if (event.mouseWheel.delta < 0) {
					_zoomCamera(1.5f);
				}
				else
				{
					_zoomCamera(0.5f);
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
				}
				break;
			}
		}

		// Turn over everything
		_draw();

		// FPS calculation
		time = _clock.getElapsedTime();
		_fps = 1.0f / time.asSeconds();
		if (skippedFrames > 5000) {
			skippedFrames = 0;
			_logger->trace("FPS: {}", _fps);
		}
		skippedFrames++;
	}
}

void Game::_initGraphics() {
	int windowStyle = sf::Style::Default;
	sf::VideoMode videoMode(_windowWidth, _windowHeight);
	if (_isFullscreen) {
		videoMode = videoMode.getDesktopMode();
		windowStyle = sf::Style::Fullscreen;
	};
	_window = make_unique<sf::RenderWindow>(videoMode, GAME_NAME, windowStyle);
	sf::View v = _window->getView();
	v.setCenter(_assetRegistry->getMap(MAP_NAME).getCenter());
	_window->setView(v);
	_window->setMouseCursorVisible(true);
	_isMovingCamera = false;
	_curCameraZoom = 1.0f;
	_font.loadFromFile("assets/fonts/tahoma.ttf");
	_mouseCoordsString.setFont(_font);
}

void Game::_loadAssets() {
	if (!_assetRegistry) {
		_assetRegistry = make_unique<Archipelago::AssetRegistry>();
	}
	_assetRegistry->prepareGoodsAtlas();
	// map must be loaded last, because it needs other assets, such as goods
	_assetRegistry->loadMap(MAP_NAME, "assets/maps/default_map.json");
	_prevTile = nullptr;
}

void Game::_draw() {
	_window->clear();

	// Render world
	_assetRegistry->getMap(MAP_NAME).draw(*_window);

	// Render HUD
	sf::View v = _window->getView();
	_window->setView(_window->getDefaultView());
	_window->draw(_mouseCoordsString);
	_window->setView(v);

	// Display everything
	_window->display();
}

void Game::_moveCamera(float offsetX, float offsetY) {
	sf::View v = _window->getView();
	sf::Vector2f viewCenter = v.getCenter();
	viewCenter.x = viewCenter.x + offsetX;
	viewCenter.y = viewCenter.y + offsetY;
	sf::Vector2f w = _assetRegistry->getMap(MAP_NAME).screenToMapCoords(viewCenter);
	if (w.x < 0 || w.y < 0 || w.x > _assetRegistry->getMap(MAP_NAME).getMapWidth() || w.y > _assetRegistry->getMap(MAP_NAME).getMapHeight()) {
		return;
	}
	v.move(offsetX, offsetY);
	_window->setView(v);
}

void Game::_zoomCamera(float zoomFactor) {
	if (_curCameraZoom * zoomFactor > MAX_CAMERA_ZOOM || _curCameraZoom * zoomFactor < MIN_CAMERA_ZOOM) {
		return;
	}
	sf::View v = _window->getView();
	v.zoom(zoomFactor);
	_window->setView(v);
	_curCameraZoom *= zoomFactor;
}

void Game::_updateMousePositionString() {
	sf::Vector2f s = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
	sf::Vector2f w = _assetRegistry->getMap(MAP_NAME).screenToMapCoords(s);
	std::string str = "Screen X: " + std::to_string(s.x) + " Y: " + std::to_string(s.y) + "; ";
	str += "World X:" + std::to_string(w.x) + " Y: " + std::to_string(w.y) + "; ";
	_mouseCoordsString.setString(str);

	Tile* tile = _assetRegistry->getMap(MAP_NAME).getTileAt(static_cast<int>(floor(w.x)), static_cast<int>(floor(w.y)));
	if (tile) {
		if (_prevTile) {
			_prevTile->getSprite().setColor(sf::Color::White);
		}
		tile->getSprite().setColor(sf::Color(255, 255, 255, 127));
		_prevTile = tile;
	}
}