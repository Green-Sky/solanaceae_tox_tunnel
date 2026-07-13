#pragma once

#include <solanaceae/contact/fwd.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/handle.hpp>

#include "./connection_type.hpp"

#include <string>

namespace Tunnel::Components {

	// generic infos required to construct the net backend
	struct MappingInfo {
		ConnectionType type;
		uint32_t id;
		std::string name;
	};

	struct ConnAddrNet {
		int socket;
		std::string addr; // src/dst (empty for streams)
	};

	struct ConnAddrSol {
		ContactHandle4 c;
		uint32_t mapping_id;
		int con_id; // ??
	};

	struct TagUDP {}; // datagram only
	struct TagTCP {}; // stream only
	struct TagUDS {}; // unix domain socket

} // Tunnel::Components

struct TunnelRegistry {
	// for connections, effectivly bi directional mapping between the (local) network and the contact
	entt::registry _reg;

};

