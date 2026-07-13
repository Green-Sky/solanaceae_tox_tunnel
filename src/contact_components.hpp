#pragma once

#include "./connection_type.hpp"

#include <cstdint>
#include <string>
#include <map>

namespace Contact::Components::ToxTunnel {

struct ReceivedMappings {
	struct Mapping {
		ConnectionType type;
		uint32_t id;
		std::string name;
		uint64_t ts;
	};

	std::map<uint32_t, Mapping> mappings;
};

} // Contact::Components::ToxTunnel
