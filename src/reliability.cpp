#include "./reliability.hpp"

void Reliability::sendData(uint16_t& seq, ByteSpan data) {
	seq = next_seq++;
	unacked_data[seq] = std::vector<uint8_t>{data};
}

void Reliability::receiveAck(uint16_t seq) {
	if (auto it = unacked_data.find(seq); it != unacked_data.cend()) {
		unacked_data.erase(it);
	}
}

// a > b, accounting for overflow wraparound
static bool seq_gt(uint16_t a, uint16_t b) {
	return
		((a > b) && (a - b <= 32768)) ||
		((a < b) && (b - a  > 32768))
	;
}

// TODO: do i need lt?

void Reliability::receiveData(uint16_t seq, ByteSpan data) {
	if (!seq_gt(seq, last_remote_seq)) {
		return; // discard, already have
		// hmmmmmmmmmmmmmm, still need to ack
		// TODO: dedup
		acks.push_back(seq);
	}

	// TODO: dedup
	acks.push_back(seq);
	incomming_data[seq] = std::vector<uint8_t>{data};
}

bool Reliability::popData(std::vector<uint8_t>& data) {
	auto it = incomming_data.find(last_remote_seq+1);
	if (it == incomming_data.cend()) {
		return false;
	}

	data = std::move(it->second);
	incomming_data.erase(it);
	last_remote_seq += 1;
	return true;
}

