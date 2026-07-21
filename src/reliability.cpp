#include "./reliability.hpp"

#include <solanaceae/util/time.hpp>

void Reliability::sendData(uint16_t& seq, ByteSpan data) {
	seq = next_seq++;
	unacked_data[seq].data = std::vector<uint8_t>{data};
	unacked_data[seq].ts = getTimeMS();
}

void Reliability::receiveAck(uint16_t seq) {
	auto it = unacked_data.find(seq);
	if (it == unacked_data.cend()) {
		return;
	}

	if (!it->second.resent) {
		const float delta_ms = (getTimeMS() - it->second.ts) / 1000.f;
		if (send_rtt_ema == 0.f) {
			send_rtt_ema = delta_ms;
		} else {
			// lerp, resulting in exponential moving average
			send_rtt_ema = send_rtt_ema * 0.98f + delta_ms * 0.02f;
		}
	}

	unacked_data.erase(it);
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
		// still need to ack
		acks.push_back({seq, 0});
		return; // discard, already have
	}

	// TODO: dedup?
	acks.push_back({seq, 0});
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

