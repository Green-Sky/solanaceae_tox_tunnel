#pragma once

#include <variant>
#include <string>
#include <cstdint>

#include <solanaceae/util/span.hpp>

struct Address {
	struct IP4Port {
		union {
			uint32_t v_32;
			uint8_t v_8[4];
		} ip {uint32_t(0)};
		uint16_t port{0};
	};
	struct IP6Port {
		union {
			uint8_t uint8[16];
			uint64_t uint64[2];
		} ip {{uint64_t(0), uint64_t(0)}};
		uint16_t port{0};
	};

	// ipv4, ipv6 or arbitrary string
	std::variant<IP4Port, IP6Port, std::string> addr;

};

Address fromStringToIP(const Address& addr);

struct SocketUDP {
	~SocketUDP(void);

	bool send(const Address& addr, ByteSpan data);
};
