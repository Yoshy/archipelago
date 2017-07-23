#include "globals.h"
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <thread>
#include "game.h"
#include "asset_registry.h"

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

	while (_window->isOpen())
	{
		_clock.restart();

		// Events processing
		sf::Event event;
		while (_window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				_window->close();
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
}

void Game::_loadAssets() {
	if (!_assetRegistry) {
		_assetRegistry = make_unique<Archipelago::AssetRegistry>();
	}
	_assetRegistry->loadAssetFromFile(AssetType::Map, "map1", "assets/maps/test_map2.json");
}

void Game::_draw() {
	_window->clear();

	_assetRegistry->getMap("map1").draw(*_window);

	_window->display();
}
