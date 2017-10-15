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
	// UI constants
	const float ui_MainInterfaceWindowWidth{ 200 };
	const float ui_MainInterfaceWindowHeight{ 200 };
	const float ui_StatusBarHeight{ 38 };
	const std::string& ui_TopStatusBar_TimeLabelId{ "tsb_time_label" };
	const std::string& ui_TopStatusBar_GoodsLabelId{ "tsb_wares_label" };
	const std::string& ui_BottomStatusBar_LabelId{ "bsb_label" };
	const float maxCameraZoom{ 3.0f };
	const float minCameraZoom{ 0.2f };
	// Time constants
	const unsigned int gameMonthDurationNormal{ 30 };
	const unsigned int gameMonthDurationFast{ 10 };
	const unsigned int gameMonthDurationSuperFast{ 1 };

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
	sf::Time frameTime;
	sf::Time accumulatedTime;
	std::string mousePositionString;
	std::string statusString;

	mousePositionString.reserve(stringReservationSize);
	statusString.reserve(stringReservationSize);
	accumulatedTime = sf::Time::Zero;
	while (_window->isOpen()) {
		// Events processing
		sf::Event event;
		while (_window->pollEvent(event)) {
			_uiDesktop->HandleEvent(event);
			switch (event.type) {
			case sf::Event::Resized:
				_resizeUi( static_cast<float>(event.size.width), static_cast<float>(event.size.height) );
				break;
			case sf::Event::KeyPressed:
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					_window->close();
					break;
				case sf::Keyboard::A:
					_moveCamera(-static_cast<float>(_assetRegistry->getMap(_curMapName).getTileWidth()), 0);
					break;
				case sf::Keyboard::D:
					_moveCamera(static_cast<float>(_assetRegistry->getMap(_curMapName).getTileWidth()), 0);
					break;
				case sf::Keyboard::W:
					_moveCamera(0, -static_cast<float>(_assetRegistry->getMap(_curMapName).getTileHeight()));
					break;
				case sf::Keyboard::S:
					_moveCamera(0, static_cast<float>(_assetRegistry->getMap(_curMapName).getTileHeight()));
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
					//_showTerrainInfo();
				}
				break;
			}
		}

		// Keyboard state processing
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			_assetRegistry->getMap(_curMapName).setWaresVisibility(true);
		}
		else {
			_assetRegistry->getMap(_curMapName).setWaresVisibility(false);
		}

		// Status string composition
		// TODO: »з-за такого тупого ежекадрового переписывани€ строк очень сильно просаживаетс€ FPS. Ќеобходимо делать это только при изменении соответствующих значений (event-based)
		frameTime = _clock.getElapsedTime();
		accumulatedTime += frameTime;
		if (accumulatedTime.asSeconds() >= _currentGameMonthDuration) {
			_gameTime++;
			accumulatedTime = sf::Time::Zero;
		}
		_clock.restart();
		_fps = static_cast<int>(1.0f / frameTime.asSeconds());
		statusString = mousePositionString;
		statusString += " FPS: ";
		statusString += std::to_string(_fps);
		std::dynamic_pointer_cast<sfg::Label>(_uiBottomStatusBar->GetWidgetById(ui_BottomStatusBar_LabelId))->SetText(statusString);

		std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(ui_TopStatusBar_TimeLabelId))->SetText(_getCurrentGameTimeString());
		for (unsigned int i = 0; i < _settlementGoods.size(); i++) {
			auto s = ui_TopStatusBar_GoodsLabelId + std::to_string(i);
			std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(s))->SetText(std::to_string(_settlementGoods[i].amount));
		};

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

		// Turn over everything
		_uiDesktop->Update(frameTime.asSeconds());
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
	_window = std::make_unique<sf::RenderWindow>(videoMode, gameName, windowStyle);
	_window->setVerticalSyncEnabled(_enable_vsync);

	// Init viewport
	sf::View v = _window->getView();
	v.setCenter(_assetRegistry->getMap(_curMapName).getCenter());
	_window->setView(v);
	_window->setMouseCursorVisible(true);
	_isMovingCamera = false;
	_curCameraZoom = 1.0f;

	// Init UI
	_sfgui = std::make_unique<sfg::SFGUI>();
	_uiDesktop = std::make_unique<sfg::Desktop>();

	_uiTopStatusBar = sfg::Window::Create();
	_uiTopStatusBar->SetStyle(sfg::Window::BACKGROUND);
	auto currentGameTimeLabel = sfg::Label::Create();
	currentGameTimeLabel->SetId(ui_TopStatusBar_TimeLabelId);
	_uiDesktop->Add(_uiTopStatusBar);
	auto mainBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 0.0f);
	auto waresBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 0.0f);
	waresBox->SetSpacing(10.0f);
	for (unsigned int i = 0; i < _settlementGoods.size(); i++) {
		auto waresIcon = sfg::Image::Create(_assetRegistry->getWaresSpecification(_settlementGoods[i].type).icon->copyToImage()); // ”дал€ем гланды через ж? ƒа.
		auto waresAmountText = sfg::Label::Create(std::to_string(_settlementGoods[i].amount));
		auto spacer = sfg::Label::Create("  ");
		waresAmountText->SetAlignment(sf::Vector2f(0.0f, 0.5f));
		waresAmountText->SetId(ui_TopStatusBar_GoodsLabelId + std::to_string(i));
		waresBox->Pack(waresIcon, false, true);
		waresBox->Pack(waresAmountText, false, true);
		waresBox->Pack(spacer, false, true);
	}
	mainBox->Pack(waresBox);
	mainBox->Pack(currentGameTimeLabel);
	_uiTopStatusBar->Add(mainBox);

	_uiBottomStatusBar = sfg::Window::Create();
	_uiBottomStatusBar->SetStyle(sfg::Window::BACKGROUND);
	auto _uiBottomStatusBarLabel = sfg::Label::Create();
	_uiBottomStatusBarLabel->SetId(ui_BottomStatusBar_LabelId);
	_uiBottomStatusBar->Add(_uiBottomStatusBarLabel);
	_uiDesktop->Add(_uiBottomStatusBar);

	_uiMainInterfaceWindow = sfg::Window::Create();
	_uiMainInterfaceWindow->Show(true);
	_uiMainInterfaceWindow->SetStyle(sfg::Window::BACKGROUND);
	//_uiMainInterfaceWindow->GetSignal(sfg::Window::OnMouseEnter).Connect(std::bind([this] { _onTerrainInfoWindowMouseEnter(); }));
	//_uiMainInterfaceWindow->GetSignal(sfg::Window::OnMouseLeave).Connect(std::bind([this] { _onTerrainInfoWindowMouseLeave(); }));
	auto mainNotebook = sfg::Notebook::Create();
	auto buildBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);

	sf::Image buildingIconImg;
	auto buildingBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	buildBox->Pack(buildingBox, false);
	if (buildingIconImg.loadFromFile("assets/textures/building_base_camp_1.png")) {
		buildingBox->Pack(sfg::Image::Create(buildingIconImg), false);
		buildingBox->Pack(sfg::Label::Create("Base camp"), false);
	}
	buildingBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	buildBox->Pack(buildingBox, false);
	if (buildingIconImg.loadFromFile("assets/textures/building_woodcutter_1.png")) {
		buildingBox->Pack(sfg::Image::Create(buildingIconImg), false);
		buildingBox->Pack(sfg::Label::Create("Woodcutter"), false);
	}

	auto infoBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
	mainNotebook->AppendPage(buildBox, sfg::Label::Create("Build"));
	mainNotebook->AppendPage(infoBox, sfg::Label::Create("Info"));
	_uiMainInterfaceWindow->Add(mainNotebook);
	_uiDesktop->Add(_uiMainInterfaceWindow);
	_resizeUi(static_cast<float>(_windowWidth), static_cast<float>(_windowHeight));
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

std::string Game::_getCurrentGameTimeString() {
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
		_settlementGoods.push_back(std::move(stack));
	}
}

void Game::_draw() {
	_window->clear();

	// Render world
	_assetRegistry->getMap(_curMapName).draw(*_window);

	// Display everything
	_sfgui->Display(*_window);
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

void Game::_zoomCamera(float zoomFactor) {
	if (_curCameraZoom * zoomFactor > maxCameraZoom || _curCameraZoom * zoomFactor < minCameraZoom) {
		return;
	}
	sf::View v = _window->getView();
	v.zoom(zoomFactor);
	_window->setView(v);
	_curCameraZoom *= zoomFactor;
}

void Game::_getMousePositionString(std::string& str) {
	sf::Vector2f screenCoords = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
	sf::Vector2f mapCoords = _assetRegistry->getMap(_curMapName).screenToMapCoords(screenCoords);
	str = "Screen X: " + std::to_string(screenCoords.x) + " Y: " + std::to_string(screenCoords.y) + "; ";
	str += "World X:" + std::to_string(mapCoords.x) + " Y: " + std::to_string(mapCoords.y) + "; ";
}

void Game::_resizeUi(float width, float height) {
	_uiTopStatusBar->SetAllocation( sf::FloatRect(0, 0, width, ui_StatusBarHeight) );
	std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(ui_TopStatusBar_TimeLabelId))->SetAlignment(sf::Vector2f(1.0f, 0.0f));

	_uiBottomStatusBar->SetAllocation( sf::FloatRect(0, height - ui_StatusBarHeight, width, ui_StatusBarHeight) );
	std::dynamic_pointer_cast<sfg::Label>(_uiBottomStatusBar->GetWidgetById(ui_BottomStatusBar_LabelId))->SetAlignment(sf::Vector2f(0.0f, 0.0f));

	_uiMainInterfaceWindow->SetAllocation( sf::FloatRect(0, ui_StatusBarHeight, 0/*ui_MainInterfaceWindowWidth*/, height - 2.0f * ui_StatusBarHeight) );

	sf::View v = _window->getView();
	v.setSize(width, height);
	_window->setView(v);
}

void Game::_showTerrainInfo() {
	if (_uiMainInterfaceWindow->GetState() == sfg::Window::State::PRELIGHT) {
		_uiMainInterfaceWindow->Show(false);
		return;
	}
	sf::Vector2f screenCoords = _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
	Archipelago::Map& map = _assetRegistry->getMap(_curMapName);
	sf::Vector2f mapCoords = map.screenToMapCoords(screenCoords);
	Tile* tile = map.getTileAt(static_cast<int>(floor(mapCoords.x)), static_cast<int>(floor(mapCoords.y)));
	if (tile) {
		sf::Vector2i windowOrigin = _window->mapCoordsToPixel(tile->getSprite().getPosition());
		_uiMainInterfaceWindow->SetPosition(sf::Vector2f(windowOrigin.x + static_cast<float>(map.getTileWidth()), windowOrigin.y + static_cast<float>(map.getTileHeight())));
		_uiMainInterfaceWindow->Show(true);
	}
	else {
		_uiMainInterfaceWindow->Show(false);
	}
}
