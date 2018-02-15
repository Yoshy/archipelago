#pragma once

#include <ECS.h>
#include "game.h"
#include "tile_component.h"
#include "events.h"

using namespace ECS;

namespace Archipelago {

	extern const std::string& loggerName;

	class MapSystem : public EntitySystem,
		public EventSubscriber<MoveCameraEvent>,
		public EventSubscriber<LoadMapEvent>,
		public EventSubscriber<MoveCameraToMapCenterEvent>,
		public EventSubscriber<ZoomCameraEvent> {
	public:
		MapSystem(Game& game) : _game(game), _mapWidth(0), _mapHeight(0), _tileWidth(0), _tileHeight(0) {};
		virtual ~MapSystem() {};
		virtual void configure(World* world) override;
		virtual void unconfigure(World* world) override;
		virtual void tick(World* world, float deltaTime) override {
			world->each<TileComponent>([&](Entity* ent, ComponentHandle<TileComponent> tile) {
				_game.getRenderWindow().draw(tile->_sprite);
			});
		}
		virtual void receive(World* world, const LoadMapEvent& event) override;
		virtual void receive(World* world, const MoveCameraEvent& event) override;
		virtual void receive(World* world, const ZoomCameraEvent& event) override;
		virtual void receive(World* world, const MoveCameraToMapCenterEvent& event) override;

		const sf::Vector2f mapToScreenCoords(sf::Vector2f mapCoords);
		const sf::Vector2f screenToMapCoords(sf::Vector2f screenCoords);

		unsigned int _getMapWidth();
		unsigned int _getMapHeight();
	private:
		Game& _game;
		unsigned int _mapWidth;
		unsigned int _mapHeight;
		unsigned int _tileWidth;
		unsigned int _tileHeight;
		float _curCameraZoom;
	};

} // namespace Archipelago
