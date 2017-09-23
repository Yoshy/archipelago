#include "globals.h"
#include <fstream>
#include <thread>
#include "game.h"
#include "asset_registry.h"
#include <SFML/Window.hpp>
#include <cmath>
#include "json.hpp"

using namespace Archipelago;
using namespace std;
using namespace spdlog;

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
	_logger->trace("Logging level set to " + to_string(logLevel));
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
	_loadAssets();
	_initGraphics();
}

void Game::shutdown() {
	_logger->info("** Archipelago finishing **");
	_logger->flush();
}

void Game::run() {
	sf::Time time;
	std::string mousePositionString;
	std::string statusString;
	unsigned int skippedFrames = 0;

	mousePositionString.reserve(STRING_RESERVATION_SIZE);
	statusString.reserve(STRING_RESERVATION_SIZE);
	while (_window->isOpen()) {
		// Events processing
		sf::Event event;
		while (_window->pollEvent(event)) {
			_uiDesktop->HandleEvent(event);
			switch (event.type) {
			case sf::Event::KeyPressed:
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					_window->close();
					break;
				case sf::Keyboard::A:
					_moveCamera(-static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileWidth()), 0);
					break;
				case sf::Keyboard::D:
					_moveCamera(static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileWidth()), 0);
					break;
				case sf::Keyboard::W:
					_moveCamera(0, -static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileHeight()));
					break;
				case sf::Keyboard::S:
					_moveCamera(0, static_cast<float>(_assetRegistry->getMap(MAP_NAME).getTileHeight()));
					break;
				}
			case sf::Event::MouseMoved:
				if (_isMovingCamera) {
					_moveCamera(static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).x),
						        static_cast<float>((_prevMouseCoords - sf::Mouse::getPosition(*_window)).y));
					_prevMouseCoords = sf::Mouse::getPosition(*_window);
				}
				_getMousePositionString(mousePositionString);
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
				};
				if (event.mouseButton.button == sf::Mouse::Left) {
					_showTerrainInfo();
				}
				break;
			}
		}

		// Keyboard state processing
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			_assetRegistry->getMap(MAP_NAME).setGoodsVisibility(true);
		}
		else {
			_assetRegistry->getMap(MAP_NAME).setGoodsVisibility(false);
		}

		// Status string composition
		time = _clock.getElapsedTime();
		_clock.restart();
		_fps = static_cast<int>(1.0f / time.asSeconds());
		statusString = mousePositionString;
		statusString += " FPS: ";
		statusString += to_string(_fps);
		_uiStatusBar->SetText(statusString);

		sf::Vector2f screenCoords = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
		sf::Vector2f mapCoords = _assetRegistry->getMap(MAP_NAME).screenToMapCoords(screenCoords);
		Tile* tile = _assetRegistry->getMap(MAP_NAME).getTileAt(static_cast<int>(floor(mapCoords.x)), static_cast<int>(floor(mapCoords.y)));
		if (tile) {
			if (_prevTile) {
				_prevTile->getSprite().setColor(sf::Color::White);
			}
			tile->getSprite().setColor(sf::Color(255, 255, 255, 127));
			_prevTile = tile;
		}

		// Turn over everything
		_uiDesktop->Update(time.asSeconds());
		_draw();

	}
}

void Game::_initGraphics() {
	// Init render window
	int windowStyle = sf::Style::Default;
	sf::VideoMode videoMode(_windowWidth, _windowHeight);
	if (_isFullscreen) {
		videoMode = videoMode.getDesktopMode();
		windowStyle = sf::Style::Fullscreen;
	};
	_window = make_unique<sf::RenderWindow>(videoMode, GAME_NAME, windowStyle);
	_window->setVerticalSyncEnabled(_enable_vsync);

	// Init viewport
	sf::View v = _window->getView();
	v.setCenter(_assetRegistry->getMap(MAP_NAME).getCenter());
	_window->setView(v);
	_window->setMouseCursorVisible(true);
	_isMovingCamera = false;
	_curCameraZoom = 1.0f;

	// Init UI
	_sfgui = make_unique<sfg::SFGUI>();
	_uiDesktop = make_unique<sfg::Desktop>();
	_uiMainWindow = sfg::Window::Create();
	_uiMainWindow->SetRequisition(sf::Vector2f(UI_MAIN_WINDOW_WIDTH, static_cast<float>(_window->getSize().y)));
	_uiMainWindow->SetStyle(sfg::Window::BACKGROUND);
	_uiDesktop->Add(_uiMainWindow);

	_uiStatusBar = sfg::Label::Create("Archipelago");
	_uiStatusBar->SetPosition(sf::Vector2f(UI_MAIN_WINDOW_WIDTH + 10, static_cast<float>(_window->getSize().y - _uiStatusBar->GetRequisition().y)));
	_uiStatusBar->SetRequisition(sf::Vector2f(static_cast<float>(_window->getSize().y) - UI_MAIN_WINDOW_WIDTH, _uiStatusBar->GetRequisition().y));
	_uiStatusBar->SetAlignment(sf::Vector2f(1.0f, 0.5f));
	_uiDesktop->Add(_uiStatusBar);

	_uiTerrainInfoWindow = sfg::Window::Create();
	_uiTerrainInfoWindow->Show(false);
	_uiTerrainInfoWindow->SetTitle("Terrain information");
	_uiTerrainInfoWindow->SetStyle(sfg::Window::BACKGROUND | sfg::Window::TITLEBAR | sfg::Window::SHADOW);
	_uiTerrainInfoWindow->SetRequisition(sf::Vector2f(UI_TERRAIN_INFO_WINDOW_WIDTH, UI_TERRAIN_INFO_WINDOW_HEIGHT));
	_uiTerrainInfoWindow->GetSignal(sfg::Window::OnMouseEnter).Connect(std::bind([this] { _onTerrainInfoWindowMouseEnter(); }));
	_uiTerrainInfoWindow->GetSignal(sfg::Window::OnMouseLeave).Connect(std::bind([this] { _onTerrainInfoWindowMouseLeave(); }));
	_uiDesktop->Add(_uiTerrainInfoWindow);
}

void Game::_loadAssets() {
	if (!_assetRegistry) {
		_assetRegistry = make_unique<Archipelago::AssetRegistry>();
	}
	_assetRegistry->prepareGoodsAtlas();
	// map must be loaded last, because it needs other assets, such as goods
	_assetRegistry->loadMap(MAP_NAME, "assets/maps/" MAP_NAME ".json");
	_prevTile = nullptr;
}

void Game::_draw() {
	_window->clear();

	// Render world
	_assetRegistry->getMap(MAP_NAME).draw(*_window);

	// Display everything
	_sfgui->Display(*_window);
	_window->display();
}

void Game::_moveCamera(float offsetX, float offsetY) {
	sf::View v = _window->getView();
	sf::Vector2f viewCenter = v.getCenter();
	viewCenter.x = viewCenter.x + offsetX;
	viewCenter.y = viewCenter.y + offsetY;
	sf::Vector2f mapCoords = _assetRegistry->getMap(MAP_NAME).screenToMapCoords(viewCenter);
	if (mapCoords.x < 0 || mapCoords.y < 0 || mapCoords.x > _assetRegistry->getMap(MAP_NAME).getMapWidth() || mapCoords.y > _assetRegistry->getMap(MAP_NAME).getMapHeight()) {
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

void Game::_getMousePositionString(std::string& str) {
	sf::Vector2f screenCoords = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
	sf::Vector2f mapCoords = _assetRegistry->getMap(MAP_NAME).screenToMapCoords(screenCoords);
	str = "Screen X: " + std::to_string(screenCoords.x) + " Y: " + std::to_string(screenCoords.y) + "; ";
	str += "World X:" + std::to_string(mapCoords.x) + " Y: " + std::to_string(mapCoords.y) + "; ";
}

void Game::_showTerrainInfo() {
	if (_uiTerrainInfoWindow->GetState() == sfg::Window::State::PRELIGHT) {
		_uiTerrainInfoWindow->Show(false);
		return;
	}
	sf::Vector2f screenCoords = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
	Archipelago::Map& map = _assetRegistry->getMap(MAP_NAME);
	sf::Vector2f mapCoords = map.screenToMapCoords(screenCoords);
	Tile* tile = map.getTileAt(static_cast<int>(floor(mapCoords.x)), static_cast<int>(floor(mapCoords.y)));
	if (tile) {
		sf::Vector2i windowOrigin = _window->mapCoordsToPixel(tile->getSprite().getPosition());
		_uiTerrainInfoWindow->SetPosition(sf::Vector2f(windowOrigin.x + static_cast<float>(map.getTileWidth()), windowOrigin.y + static_cast<float>(map.getTileHeight())));
		_uiTerrainInfoWindow->Show(true);
	}
	else {
		_uiTerrainInfoWindow->Show(false);
	}
}

void Game::_onTerrainInfoWindowMouseEnter() {
	_uiTerrainInfoWindow->SetState(sfg::Window::State::PRELIGHT);
}

void Game::_onTerrainInfoWindowMouseLeave() {
	_uiTerrainInfoWindow->SetState(sfg::Window::State::NORMAL);
}
