#pragma once

#include "building_specification.h"

namespace Archipelago {
	struct BuildingComponent {
		BuildingComponent(const BuildingSpecification* _spec) : spec(_spec) {};
		sf::Sprite sprite;
		const BuildingSpecification* spec;
	};
} // namespace Archipelago