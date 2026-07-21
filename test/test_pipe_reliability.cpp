#include <reliability.hpp>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <random>
#include <deque>

static void test_simple_roundtrip(void) {
	Reliability rel_alice;

	uint16_t pkg1_seq;
	const std::vector<uint8_t> pkg1_data{0x00, 0x0f, 0x00, 0xef};
	rel_alice.sendData(pkg1_seq, ByteSpan{pkg1_data});
	assert(pkg1_seq == 0);
	assert(rel_alice.next_seq == 1);
	assert(rel_alice.unacked_data.size() == 1);
	assert(rel_alice.unacked_data.count(pkg1_seq) == 1);
	assert(rel_alice.unacked_data.at(pkg1_seq).data == pkg1_data);

	Reliability rel_bob;
	rel_bob.receiveData(pkg1_seq, ByteSpan{pkg1_data});
	assert(rel_bob.acks.size() == 1);
	assert(rel_bob.acks.front().seq == pkg1_seq);
	assert(rel_bob.incomming_data.size() == 1);

	rel_alice.receiveAck(pkg1_seq);
	assert(rel_alice.unacked_data.empty());

	{
		std::vector<uint8_t> rdata;
		const bool ret = rel_bob.popData(rdata);
		assert(ret);
		assert(rel_bob.incomming_data.empty());
		assert(rel_bob.last_remote_seq == pkg1_seq);
		assert(rdata == pkg1_data);
	}

	printf("test_simple_roundtrip: PASSED\n");
}

// todo receive only tests
// - recieve seq less or equal
// - already received seq
// - receive out of order
// - seq missing

static void test_pop_empty(void) {
	Reliability rel;
	std::vector<uint8_t> rdata;
	const bool ret = rel.popData(rdata);
	assert(!ret);
	assert(rdata.empty());
	assert(rel.incomming_data.empty());
	assert(rel.last_remote_seq == (uint16_t)-1);

	printf("test_pop_empty: PASSED\n");
}

static void test_complex_roundtrip_loop(void) {
	Reliability rel_alice;
	Reliability rel_bob;

	std::map<uint16_t, std::vector<uint8_t>> sent_by_alice;
	std::map<uint16_t, std::vector<uint8_t>> sent_by_bob;

	std::minstd_rand rng{1337*11};

	for (size_t i = 0; i < 50; i++) {
		{ // do alice data
			// first receive
			std::vector<uint8_t> rdata;
			while (rel_alice.popData(rdata)) {
				assert(rdata == sent_by_bob.at(rel_alice.last_remote_seq));
			}

			// now sending
			while (rng()%256 > 100) {
				size_t size = rng()%2000;
				std::vector<uint8_t> sdata(size);
				for (uint8_t& it : sdata) { it = rng(); }

				uint16_t new_seq;
				rel_alice.sendData(new_seq, ByteSpan{sdata});
				rel_bob.receiveData(new_seq, ByteSpan{sdata});
				sent_by_alice[new_seq] = sdata;
			}
		}

		{ // do bob data
			// first receive
			std::vector<uint8_t> rdata;
			while (rel_bob.popData(rdata)) {
				assert(rdata == sent_by_alice.at(rel_bob.last_remote_seq));
			}

			// now sending
			while (rng()%256 > 100) {
				size_t size = rng()%2000;
				std::vector<uint8_t> sdata(size);
				for (uint8_t& it : sdata) { it = rng(); }

				uint16_t new_seq;
				rel_bob.sendData(new_seq, ByteSpan{sdata});
				rel_alice.receiveData(new_seq, ByteSpan{sdata});
				sent_by_bob[new_seq] = sdata;
			}
		}

		{ // do alice ack receiving
			for (auto it = rel_bob.acks.cbegin(); it != rel_bob.acks.cend();) {
				if (rng()%10 == 0) {
					break;
				}

				rel_alice.receiveAck(it->seq);
				it = rel_bob.acks.erase(it);
			}
		}

		{ // do bob ack receiving
			for (auto it = rel_alice.acks.cbegin(); it != rel_alice.acks.cend();) {
				if (rng()%10 == 0) {
					break;
				}

				rel_bob.receiveAck(it->seq);
				it = rel_alice.acks.erase(it);
			}
		}
	}

	printf("test_complex_roundtrip_loop: PASSED\n");
}

int main(void) {
	test_simple_roundtrip();
	test_pop_empty();
	test_complex_roundtrip_loop();

	printf("\nAll tests passed!\n");
	return 0;
}
