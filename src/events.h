#pragma once

struct LoadMapEvent {
	std::string& filename;
};

struct MoveCameraEvent {
	float offsetX;
	float offsetY;
};

struct MoveCameraToMapCenterEvent {
	int foo;
};

struct ZoomCameraEvent {
	float zoomFactor;
};
