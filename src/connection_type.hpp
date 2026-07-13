#pragma once

#include <cstdint>

enum class ConnectionType : uint8_t {
	STREAM_LISTEN,
	//STREAM_CONNECT, // TODO: reverse connections
	DATAGRAM_LISTEN,
};
