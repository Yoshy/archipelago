#pragma once

#include <SFGUI/Widgets.hpp>
#include "building_specification.h"

namespace Archipelago {

	class Game;

	class UiBuildingTipWindow {
	public:
		UiBuildingTipWindow(Archipelago::Game* game);
		sfg::Window::Ptr getSFGWindow() { return _window; };
		void onMouseLeave();
		void onMouseMove(BuildingTypeId buildingId);
	private:
		Archipelago::Game* _game;
		bool _isShown;
		BuildingTypeId _buildingId;
		sfg::Window::Ptr _window;
		void _show(bool show);
	};

} // namespace Archipelago