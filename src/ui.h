#pragma once

#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include "tile_component.h"
#include "ui_terrain_info_window.h"

namespace Archipelago {

	class Game;

	class Ui {
	public:
		Ui(Game& game);
		void render();
		void update(float seconds);
		void updateSettlementWares();
		void updateGameTimeString();
		void handleEvent(const sf::Event& event);
		void resizeUi(float width, float height);
		void showTerrainInfoWindow(sf::Vector2f position, TileComponent& tile);
		void hideTerrainInfoWindow();
	private:
		void _constructTopStatusBar();
		void _constructBottomStatusBar();
		void _constructMainInterfaceWindow();
		void _constructTerrainInfoWindow();
		Archipelago::Game& _game;
		std::unique_ptr<sfg::SFGUI> _sfgui;
		std::unique_ptr<sfg::Desktop> _uiDesktop;
		sfg::Window::Ptr _uiTopStatusBar;
		sfg::Window::Ptr _uiBottomStatusBar;
		sfg::Window::Ptr _uiMainInterfaceWindow;
		UiTerrainInfoWindow::Ptr _uiTerrainInfoWindow;
		float _fpsUpdateInterval; // seconds
		float _timeSincelastFpsUpdate; // seconds
	};
}
