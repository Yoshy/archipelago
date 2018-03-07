#pragma once

#include <cstdint>
#include "natural_resources_specification.h"

namespace Archipelago {

	/** Natural resource component
	*
	* This component contains data of available natural resources on the tile
	*
	*/
	struct NaturalResourceComponent {
		NaturalResourceComponent() : resourceSet(0) {};
		NaturalResourceComponent(NaturalResourceTypeId _type, uint32_t _resourceSet) : type(_type), resourceSet(_resourceSet) {};
		NaturalResourceTypeId type;
		uint32_t resourceSet;
	};

} // namespace Archipelago
