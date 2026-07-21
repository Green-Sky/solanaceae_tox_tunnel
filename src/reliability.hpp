#pragma once

#include <solanaceae/util/span.hpp>

#include <deque>
#include <vector>
#include <map>
#include <cstdint>

// bookkeeping
struct Reliability {
	uint16_t next_seq {0};
	struct UnackedData{
		std::vector<uint8_t> data;
		uint64_t ts;
		bool resent{false};
	};
	std::map<uint16_t, UnackedData> unacked_data;
	float send_rtt_ema {0.f}; // sec

	uint16_t last_remote_seq {(uint16_t)-1};
	std::map<uint16_t, std::vector<uint8_t>> incomming_data;
	struct AckWithCount {
		uint16_t seq;
		uint8_t send_count; // usually capped at 2
	};
	std::deque<AckWithCount> acks;

	void sendData(uint16_t& seq, ByteSpan data);
	void receiveAck(uint16_t seq);

	void receiveData(uint16_t seq, ByteSpan data);

	bool popData(std::vector<uint8_t>& data);
};

