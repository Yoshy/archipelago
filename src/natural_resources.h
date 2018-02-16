#pragma once

#include <cstdint>

namespace Archipelago {

	/** Natural resource constants
	*
	* See natural_resource_component.h for usage
	*
	*/
	const uint32_t NR_SOIL(1 << 0);
	const uint32_t NR_FOREST(1 << 1);
	const uint32_t NR_SALT_WATER(1 << 2);
	const uint32_t NR_FRESH_WATER(1 << 3);

} // namespace Archipelago
