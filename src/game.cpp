#include "game.h"
#include <spdlog\spdlog.h>
#include <fstream>
#include <rapidjson\document.h>
#include <rapidjson\error\en.h>

using namespace Apchipelago;
using namespace std;
using namespace spdlog;
using namespace rapidjson;

const char* GAME_NAME = "Archipelago";

Game::Game() {

}

Game::~Game() {
	_logger->info("** Archipelago finishing **");
	_logger->flush();
}

void Game::init() {
    _logger = basic_logger_mt("archipelago", "archipelago.log");
	_logger->set_level(level::trace);
	_logger->info("** {} starting **", GAME_NAME);
	fstream configFile;
	string configString;
	configFile.open("config.json");
	string s;
	while (configFile >> s) {
		configString += s;
	}
	configFile.close();
	Document configDOM;
	_logger->trace("Config: {}", configString.c_str());
	ParseResult pr = configDOM.Parse<kParseCommentsFlag>(configString.c_str());
	if (!pr) {
		_logger->critical("JSON parse error '{}', offset: {}", GetParseError_En(pr.Code()), pr.Offset());
	}
	try {
		_logger->set_level(static_cast<level::level_enum>(configDOM["logging"]["level"].GetInt()));

		const Value& videoSettings = configDOM["video"];
		_isFullscreen = videoSettings["fullscreen"].GetBool();
		_logger->trace("fullscreen: {}", _isFullscreen);
		_windowWidth = videoSettings["windowWidth"].GetUint();
		_logger->trace("windowWidth: {}", _windowWidth);
		_windowHeight = videoSettings["windowHeight"].GetUint();
		_logger->trace("windowHeight: {}", _windowHeight);
	}
	catch (...) {
		_logger->critical("JSON error!");
	}
	_initGraphics();
}

void Game::run() {
	sf::CircleShape shape(300.f);
	shape.setFillColor(sf::Color::Green);

	while (_window->isOpen())
	{
		sf::Event event;
		while (_window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				_window->close();
		}

		_window->clear();
		_window->draw(shape);
		_window->display();
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