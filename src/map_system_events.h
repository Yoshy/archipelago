#pragma once

struct LoadMapEvent {
	const std::string& filename;
};

struct MouseMovedEvent {
	const bool orly;
};

struct MoveCameraEvent {
	const float offsetX;
	const float offsetY;
};

struct MoveCameraToMapCenterEvent {
	const bool orly;
};

struct ConvertScreenToMapCoordsEvent {
	sf::Vector2f& coords;
};

struct ConvertMapToScreenCoordsEvent {
	sf::Vector2f& coords;
};

struct RenderMapEvent {
	const bool orly;
};

struct ShowNaturalResourcesEvent {
	const bool show;
};

struct RequestHighlightedEntityEvent {
	size_t& entityID;
};