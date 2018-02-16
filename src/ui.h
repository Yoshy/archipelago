#pragma once

#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>

namespace Archipelago {

	class Game;

	class Ui {
	public:
		Ui(Game& game);
		void render();
		void update(float seconds);
		void handleEvent(const sf::Event& event);
		void resizeUi(float width, float height);
	private:
		Archipelago::Game& _game;
		std::unique_ptr<sfg::SFGUI> _sfgui;
		std::unique_ptr<sfg::Desktop> _uiDesktop;
		sfg::Window::Ptr _uiTopStatusBar;
		sfg::Window::Ptr _uiBottomStatusBar;
		sfg::Window::Ptr _uiMainInterfaceWindow;
	};
}
