#pragma once

#include <spdlog/spdlog.h>
#include <ECS.h>
#include <SFGUI/Widgets.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "wares_specification.h"

namespace Archipelago {

	class Game;
	struct WaresSpecification;

	extern const std::string& loggerName;
	const float BUILDING_DESCRIPTION_LABEL_REQUISITION = 400.0f;

	enum class TileType { TERRAIN = 0, BUILDING = 1 };

	struct TerrainInfoWindowDataUpdateEvent {
		bool show;
		sf::Vector2f position;
		TileType tileType;
		sf::Sprite* tileSprite;
		std::string name; // Name of terrain tile or building
		std::string buildingDescription; // If BUILDING, then this value contains description of building
		const std::vector<WaresStack>* production; // If BUILDING, then this value contains produced wares
		int amount; // If BUILDING, then this value contains production amount per month
		uint32_t resourceSet; // If TERRAIN, then this value contains natural resources on tile
	};

	class UiTerrainInfoWindow : public ECS::EventSubscriber<TerrainInfoWindowDataUpdateEvent> {
	public:
		UiTerrainInfoWindow(Game* game);
		~UiTerrainInfoWindow();
		sfg::Window::Ptr getSFGWindow() { return _window; };
		void setPosition(sf::Vector2f position) { _window->SetPosition(position); };
		void show(bool show) { _window->Show(show); };
		virtual void receive(ECS::World* world, const TerrainInfoWindowDataUpdateEvent& event) override;
	private:
		void _addBuildingInfoLayout(sfg::Box::Ptr rootLayoutWidget, const TerrainInfoWindowDataUpdateEvent& event);
		void _addTerrainInfoLayout(sfg::Box::Ptr rootLayoutWidget, const TerrainInfoWindowDataUpdateEvent& event);
		Game* _game;
		sfg::Window::Ptr _window;
		sfg::Image::Ptr _hTileSprite;
		sfg::Label::Ptr _hTileName;
	};

}