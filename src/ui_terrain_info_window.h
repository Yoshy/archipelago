#pragma once

#include <spdlog/spdlog.h>
#include <ECS.h>
#include <SFGUI/Widgets.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "natural_resources_specification.h"
#include "building_specification.h"

namespace Archipelago {
	extern const std::string& loggerName;
	const float BUILDING_DESCRIPTION_LABEL_REQUISITION = 400.0f;

	enum class TileType { TERRAIN = 0, BUILDING = 1 };

	struct TerrainInfoWindowDataUpdateEvent {
		bool show;
		TileType tileType;
		sf::Sprite& tileSprite;
		std::string name; // Name of terrain tile or building
		std::string buildingDescription; // If BUILDING, then this value contains description of building
		BuildingTypeId prodType; // If BUILDING, then this value contains production type
		int amount; // If BUILDING, then this value contains production amount per month
		NaturalResourceTypeId resourceSet; // If TERRAIN, then this value contains natural resource type
	};

	class UiTerrainInfoWindow : public ECS::EventSubscriber<TerrainInfoWindowDataUpdateEvent> {
	public:
		typedef std::shared_ptr<UiTerrainInfoWindow> Ptr;
		UiTerrainInfoWindow(ECS::World* world);
		~UiTerrainInfoWindow();
		sfg::Window::Ptr getSFGWindow() { return _window; };
		void setPosition(sf::Vector2f position) { _window->SetPosition(position); };
		void show(bool show) { _window->Show(show); };
		virtual void receive(ECS::World* world, const TerrainInfoWindowDataUpdateEvent& event) override;
	private:
		void _addBuildingInfoLayout(sfg::Box::Ptr rootLayoutWidget);
		void _addTerrainInfoLayout(sfg::Box::Ptr rootLayoutWidget);
		ECS::World* _world;
		sfg::Window::Ptr _window;
		sfg::Image::Ptr _hTileSprite;
		sfg::Label::Ptr _hTileName;
		sfg::Label::Ptr _hProdTypeLabel;
		sfg::Label::Ptr _hProdAmountLabel;
	};

}