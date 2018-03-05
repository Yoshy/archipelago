#pragma once

#include <cstdint>

namespace Archipelago {

	/** Natural resource component
	*
	* This component contains data of available natural resources on the tile
	*
	*/
	struct NaturalResourceComponent {
		NaturalResourceComponent() : resourceSet(0) {};
		NaturalResourceComponent(uint32_t _resourceSet) : resourceSet(_resourceSet) {};
		uint32_t resourceSet;
	};

} // namespace Archipelago
