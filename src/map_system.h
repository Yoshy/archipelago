#pragma once

#include <ECS.h>
#include "game.h"
#include "map_system_events.h"
#include "tile_component.h"
#include "natural_resource_component.h"

using namespace ECS;

namespace Archipelago {

	extern const std::string& loggerName;

	class MapSystem : public EntitySystem,
		public EventSubscriber<LoadMapEvent>,
		public EventSubscriber<MouseMovedEvent>,
		public EventSubscriber<MoveCameraEvent>,
		public EventSubscriber<MoveCameraToMapCenterEvent>,
		public EventSubscriber<ConvertScreenToMapCoordsEvent>,
		public EventSubscriber<ConvertMapToScreenCoordsEvent>,
		public EventSubscriber<ShowNaturalResourcesEvent>,
		public EventSubscriber<RequestHighlightedEntityEvent>,
		public EventSubscriber<RenderMapEvent> {
	public:
		MapSystem(Game& game) : _game(game), _mapWidth(0), _mapHeight(0), _tileWidth(0), _tileHeight(0) {};
		virtual ~MapSystem() {};
		virtual void configure(World* world) override;
		virtual void unconfigure(World* world) override;
		virtual void tick(World* world, float deltaTime) override {};
		virtual void receive(World* world, const LoadMapEvent& event) override;
		virtual void receive(World* world, const MouseMovedEvent& event) override;
		virtual void receive(World* world, const MoveCameraEvent& event) override;
		virtual void receive(World* world, const MoveCameraToMapCenterEvent& event) override;
		virtual void receive(World* world, const ConvertScreenToMapCoordsEvent& event) override;
		virtual void receive(World* world, const ConvertMapToScreenCoordsEvent& event) override;
		virtual void receive(World* world, const ShowNaturalResourcesEvent& event) override;
		virtual void receive(World* world, const RequestHighlightedEntityEvent& event) override;
		virtual void receive(World* world, const RenderMapEvent& event) override;
	private:
		Game& _game;
		unsigned int _mapWidth;
		unsigned int _mapHeight;
		unsigned int _tileWidth;
		unsigned int _tileHeight;
		bool _showNaturalResources;
		size_t _currentHighlightedEntity;

		const sf::Vector2f _mapToScreenCoords(sf::Vector2f mapCoords);
		const sf::Vector2f _screenToMapCoords(sf::Vector2f screenCoords);
		int _numberOfSetBits(uint32_t value);
	};

} // namespace Archipelago
