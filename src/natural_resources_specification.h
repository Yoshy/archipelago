#pragma once

#include <cstdint>

namespace Archipelago {

	enum class NaturalResourceTypeId { Unknown = 0, FertileSoil = 1, Forest = 2, SaltWater = 3, FreshWater = 4, _Last = FreshWater, _First = FertileSoil };

	struct NaturalResourceSpecification {
		std::string name;
		sf::Texture* icon; // non-owning pointer
	};
} // namespace Archipelago
