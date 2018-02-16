#pragma once

#include <cstdint>

namespace Archipelago {

	/** Natural resource component
	*
	* This component contains data of available natural resources on the tile
	* _resourceMask is a bitset of available resources. See natural_resources.h for actual values
	*
	*/
	struct NaturalResourceComponent {
		NaturalResourceComponent() : _resourceMask(0) {};
		NaturalResourceComponent(uint32_t resourceMask) : _resourceMask(resourceMask) {};
		uint32_t _resourceMask;
	};

} // namespace Archipelago
