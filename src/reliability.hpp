#pragma once

#include <solanaceae/util/span.hpp>

#include <deque>
#include <vector>
#include <map>
#include <cstdint>

// bookkeeping
struct Reliability {
	uint16_t next_seq {0};
	// TODO: add send timestamp for resend
	std::map<uint16_t, std::vector<uint8_t>> unacked_data;

	uint16_t last_remote_seq {(uint16_t)-1};
	std::deque<uint16_t> acks; // always keep some
	std::map<uint16_t, std::vector<uint8_t>> incomming_data;

	void sendData(uint16_t& seq, ByteSpan data);
	void receiveAck(uint16_t seq);

	void receiveData(uint16_t seq, ByteSpan data);

	bool popData(std::vector<uint8_t>& data);
};

