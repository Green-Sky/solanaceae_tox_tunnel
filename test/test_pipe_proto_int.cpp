#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>
#include <tunnel_packets.h>
#include <reliability.hpp>

#include <solanaceae/util/time.hpp>

#include <entt/entity/registry.hpp>

#include <vector>
#include <random>

#include <cassert>

#include <unistd.h>
#include <fcntl.h>

struct FDs {
	int fd_read;
	int fd_write;
};

struct PkgWriteQueue {
	std::deque<std::vector<uint8_t>>& queue;
};
struct PkgReadQueue {
	std::deque<std::vector<uint8_t>>& queue;
};

enum PKG_TYPE : uint8_t {
	RelData,
	RelDataAck,
};

static void test_pipe_raw(void) {
	std::minstd_rand rng{11};

	entt::registry pipereg;

	auto ent_alice = pipereg.create();
	auto ent_bob = pipereg.create();

	// if rel
	pipereg.emplace<Reliability>(ent_alice);
	pipereg.emplace<Reliability>(ent_bob);

	// setup outside fds
	int pipefd_alice_in[2];
	int pipefd_alice_out[2];
	{ // alice
		// [0] is read, [1] is write
		auto ret = pipe(pipefd_alice_in);
		assert(ret == 0);
		ret = pipe(pipefd_alice_out);
		assert(ret == 0);

		fcntl(pipefd_alice_in[0], F_SETFL, fcntl(pipefd_alice_in[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_alice_in[1], F_SETFL, fcntl(pipefd_alice_in[1], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_alice_out[0], F_SETFL, fcntl(pipefd_alice_out[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_alice_out[1], F_SETFL, fcntl(pipefd_alice_out[1], F_GETFL) | O_NONBLOCK);

		pipereg.emplace<FDs>(ent_alice, pipefd_alice_in[0], pipefd_alice_out[1]);
	}

	int pipefd_bob_in[2];
	int pipefd_bob_out[2];
	{ // bob
		// [0] is read, [1] is write
		auto ret = pipe(pipefd_bob_in);
		assert(ret == 0);
		ret = pipe(pipefd_bob_out);
		assert(ret == 0);

		fcntl(pipefd_bob_in[0], F_SETFL, fcntl(pipefd_bob_in[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_bob_in[1], F_SETFL, fcntl(pipefd_bob_in[1], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_bob_out[0], F_SETFL, fcntl(pipefd_bob_out[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_bob_out[1], F_SETFL, fcntl(pipefd_bob_out[1], F_GETFL) | O_NONBLOCK);

		pipereg.emplace<FDs>(ent_bob, pipefd_bob_in[0], pipefd_bob_out[1]);
	}

	std::deque<std::vector<uint8_t>> pkg_queue_a2b;
	std::deque<std::vector<uint8_t>> pkg_queue_b2a;

	pipereg.emplace<PkgWriteQueue>(ent_alice, pkg_queue_a2b);
	pipereg.emplace<PkgWriteQueue>(ent_bob, pkg_queue_b2a);
	pipereg.emplace<PkgReadQueue>(ent_alice, pkg_queue_b2a);
	pipereg.emplace<PkgReadQueue>(ent_bob, pkg_queue_a2b);

	std::vector<uint8_t> static_data(1010);
	for (auto& it : static_data) {
		it = 0xff;
	}

	size_t counter_alice_in{0};
	size_t counter_alice_out{0};
	size_t counter_bob_in{0};
	size_t counter_bob_out{0};
	// alice in needs to be equal to bob out

	for (size_t i_outer = 0; i_outer < 100; i_outer++) {
		// pumps
		// pulls from socket and sends pkgs
		// only if pks can be sent, which is allways in this test case
		for (auto&& [e, fds, pwq] : pipereg.view<FDs, PkgWriteQueue>().each()) {
			// cap each loop to 100 reads/pkgs
			for (size_t i = 0; i < 100; i++) {
				std::vector<uint8_t> buf(1200);
				const auto rb = read(fds.fd_read, buf.data(), buf.size());
				if (rb <= 0) {
					break; // nothing more to read or something failed
				}
				buf.resize(rb);
				pwq.queue.push_back(std::move(buf));
			}
		}

		// process incoming pkgs
		for (auto&& [e, fds, prq] : pipereg.view<FDs, PkgReadQueue>().each()) {
			// do all for now
			// might need a write buffer if write fails in the future
			for (auto it = prq.queue.begin(); it != prq.queue.end();) {
				if (it->size() < 2) {
					it = prq.queue.erase(it);
					continue;
				}

				const auto rb = write(fds.fd_write, it->data(), it->size());
				if (rb <= 0) {
					break;
				} else if ((size_t)rb != it->size()) {
					// meh, partially written data
					// maybe drop if not reliable?
					std::memmove(it->data(), it->data()+rb, it->size()-rb);
					it->resize(rb);
				} else {
					it = prq.queue.erase(it);
				}
			}
		}

		// write test data
		while (i_outer < 90 && rng()%2) {
			write(pipefd_alice_in[1], static_data.data(), static_data.size());
			counter_alice_in += static_data.size();
		}
		while (i_outer < 90 && rng()%2) {
			write(pipefd_bob_in[1], static_data.data(), static_data.size());
			counter_bob_in += static_data.size();
		}

		// read and check test data
		while (true) {
			std::vector<uint8_t> buf(1200);
			const auto rb = read(pipefd_alice_out[0], buf.data(), buf.size());
			if (rb <= 0) {
				break; // nothing more to read or something failed
			}
			buf.resize(rb);
			std::printf("read %zd on alices side\n", rb);
			for (const auto it : buf) {
				assert(it == 0xff);
			}
			counter_alice_out += buf.size();
		}
		while (true) {
			std::vector<uint8_t> buf(1200);
			const auto rb = read(pipefd_bob_out[0], buf.data(), buf.size());
			if (rb <= 0) {
				break; // nothing more to read or something failed
			}
			buf.resize(rb);
			std::printf("read %zd on bobs side\n", rb);
			for (const auto it : buf) {
				assert(it == 0xff);
			}
			counter_bob_out += buf.size();
		}
	}

	std::printf("counter_alice_in: %zu\n", counter_alice_in);
	std::printf("counter_alice_out: %zu\n", counter_alice_out);
	std::printf("counter_bob_in: %zu\n", counter_bob_in);
	std::printf("counter_bob_out: %zu\n", counter_bob_out);

	assert(counter_alice_in == counter_bob_out);
	assert(counter_bob_in == counter_alice_out);
}

static void test_pipe_pkg(void) {
	std::minstd_rand rng{11};

	entt::registry pipereg;

	auto ent_alice = pipereg.create();
	auto ent_bob = pipereg.create();

	// if rel
	pipereg.emplace<Reliability>(ent_alice);
	pipereg.emplace<Reliability>(ent_bob);

	// setup outside fds
	int pipefd_alice_in[2];
	int pipefd_alice_out[2];
	{ // alice
		// [0] is read, [1] is write
		auto ret = pipe(pipefd_alice_in);
		assert(ret == 0);
		ret = pipe(pipefd_alice_out);
		assert(ret == 0);

		fcntl(pipefd_alice_in[0], F_SETFL, fcntl(pipefd_alice_in[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_alice_in[1], F_SETFL, fcntl(pipefd_alice_in[1], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_alice_out[0], F_SETFL, fcntl(pipefd_alice_out[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_alice_out[1], F_SETFL, fcntl(pipefd_alice_out[1], F_GETFL) | O_NONBLOCK);

		pipereg.emplace<FDs>(ent_alice, pipefd_alice_in[0], pipefd_alice_out[1]);
	}

	int pipefd_bob_in[2];
	int pipefd_bob_out[2];
	{ // bob
		// [0] is read, [1] is write
		auto ret = pipe(pipefd_bob_in);
		assert(ret == 0);
		ret = pipe(pipefd_bob_out);
		assert(ret == 0);

		fcntl(pipefd_bob_in[0], F_SETFL, fcntl(pipefd_bob_in[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_bob_in[1], F_SETFL, fcntl(pipefd_bob_in[1], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_bob_out[0], F_SETFL, fcntl(pipefd_bob_out[0], F_GETFL) | O_NONBLOCK);
		fcntl(pipefd_bob_out[1], F_SETFL, fcntl(pipefd_bob_out[1], F_GETFL) | O_NONBLOCK);

		pipereg.emplace<FDs>(ent_bob, pipefd_bob_in[0], pipefd_bob_out[1]);
	}

	std::deque<std::vector<uint8_t>> pkg_queue_a2b;
	std::deque<std::vector<uint8_t>> pkg_queue_b2a;

	pipereg.emplace<PkgWriteQueue>(ent_alice, pkg_queue_a2b);
	pipereg.emplace<PkgWriteQueue>(ent_bob, pkg_queue_b2a);
	pipereg.emplace<PkgReadQueue>(ent_alice, pkg_queue_b2a);
	pipereg.emplace<PkgReadQueue>(ent_bob, pkg_queue_a2b);

	size_t counter_alice_in{0};
	size_t counter_alice_out{0};
	size_t counter_bob_in{0};
	size_t counter_bob_out{0};
	// alice in needs to be equal to bob out

	for (size_t i_outer = 0; i_outer < 150; i_outer++) {
		// process incoming pkgs
		for (auto&& [e, fds, prq] : pipereg.view<FDs, PkgReadQueue>().each()) {
			// do all for now
			// might need a write buffer if write fails in the future
			for (auto it = prq.queue.begin(); it != prq.queue.end();) {
				if (it->size() < 2) {
					it = prq.queue.erase(it);
					continue;
				}

				const auto pkg_type = it->at(0);
				if (pkg_type == PKG_TYPE::RelData) {
					auto* rel_ptr = pipereg.try_get<Reliability>(e);
					assert(rel_ptr != nullptr);

					uint16_t pipe_id;
					uint16_t seq;
					const uint8_t* data;
					size_t data_len;
					{
						const auto ret = tunnel_pkg_reldata_unpack(it->data()+1, it->size()-1, &pipe_id, &seq, &data, &data_len);
						assert(ret == it->size()-1);
						assert(pipe_id == 0x33);
					}

					rel_ptr->receiveData(seq, ByteSpan{data, data_len});
					std::printf("<- received data %hu\n", seq);
				} else if (pkg_type == PKG_TYPE::RelDataAck) {
					auto* rel_ptr = pipereg.try_get<Reliability>(e);
					assert(rel_ptr != nullptr);

					uint16_t pipe_id;
					std::vector<uint16_t> seqs(100);
					size_t seqs_count{0};
					{
						const auto ret = tunnel_pkg_reldata_ack_unpack(it->data()+1, it->size()-1, &pipe_id, seqs.data(), seqs.size(), &seqs_count);
						assert(ret == it->size()-1);
						assert(pipe_id == 0x33);
						assert(seqs_count > 0);
						seqs.resize(seqs_count);
					}

					std::printf("<- received acks [");
					for (const auto seq : seqs) {
						rel_ptr->receiveAck(seq);
						std::printf("%hu,", seq);
					}
					std::printf("]\n");
					//std::printf("handled %zu acks\n", seqs.size());
				} else {
					assert(false && "unhandled pkg type");
				}

				it = prq.queue.erase(it);
			}
		}

		// send acks
		for (auto&& [e, pwq, rel] : pipereg.view<PkgWriteQueue, Reliability>().each()) {
			std::vector<uint16_t> seqs;
			for (auto it = rel.acks.begin(); it != rel.acks.end() && seqs.size() < 64;) {
				seqs.push_back(it->seq);
				if (it->send_count >= 1) { // only twice
					it = rel.acks.erase(it);
				} else {
					it->send_count += 1;
					it++;
				}
			}
			if (seqs.empty()) {
				continue;
			}

			std::vector<uint8_t> pkg_buf(2200);
			pkg_buf[0] = PKG_TYPE::RelDataAck;
			const auto bw = tunnel_pkg_reldata_ack_pack(seqs.data(), seqs.size(), 0x33, pkg_buf.data()+1, pkg_buf.size()-1);
			assert(bw != 0);
			pkg_buf.resize(1 + bw);
			if (rng()%10 != 0) { // 10% drop chance
				pwq.queue.push_back(std::move(pkg_buf));
				std::printf("-> sent acks [");
				for (const auto seq : seqs) {
					std::printf("%hu,", seq);
				}
				std::printf("]\n");
			} else {
				std::printf("-> dropped acks [");
				for (const auto seq : seqs) {
					std::printf("%hu,", seq);
				}
				std::printf("]\n");
			}
		}

		// resent unacked
		for (auto&& [e, pwq, rel] : pipereg.view<PkgWriteQueue, Reliability>().each()) {
			const uint64_t now = getTimeMS();
			// resend after ... 2 rtt ?
			for (auto& it : rel.unacked_data) {
				// default to 1 sec rtt before first ack
				// 1sec will only work in non-sim -> 10ms in sim
				// shoudl we just not if no ack? hm
				if (
					(rel.send_rtt_ema == 0.f && (now - it.second.ts)/1000.f > 0.01f) ||
					(rel.send_rtt_ema != 0.f && ((now - it.second.ts)/1000.f > rel.send_rtt_ema*2.f))
				) {
					if (rel.send_rtt_ema == 0.f) {
						std::printf("resend timer, zero rtt, %lums\n", now - it.second.ts);
					} else {
						std::printf("resend timer, %fs rtt, %lums\n", rel.send_rtt_ema, now - it.second.ts);
					}
					std::vector<uint8_t> pkg_buf(2200);
					pkg_buf[0] = PKG_TYPE::RelData;
					const auto bw = tunnel_pkg_reldata_pack(it.second.data.data(), it.second.data.size(), 0x33, it.first, pkg_buf.data()+1, pkg_buf.size()-1);
					assert(bw != 0);
					pkg_buf.resize(1 + bw);
					if (rng()%10 != 0) { // 10% drop chance
						pwq.queue.push_back(std::move(pkg_buf));
						std::printf("-> resent %hu\n", it.first);
					} else {
						std::printf("-> dropped resend %hu\n", it.first);
					}

					it.second.ts = now;
					it.second.resent = true;
				}
			}
		}

		// pumps
		// pulls from socket and sends pkgs
		// only if pks can be sent, which is allways in this test case
		// would also need to account for resends (see unacked above)
		for (auto&& [e, fds, pwq] : pipereg.view<FDs, PkgWriteQueue>().each()) {
			// cap each loop to 100 reads/pkgs
			for (size_t i = 0; i < rng()%2+1; i++) {
				std::vector<uint8_t> buf(1200);
				const auto rb = read(fds.fd_read, buf.data(), buf.size());
				if (rb <= 0) {
					break; // nothing more to read or something failed
				}
				buf.resize(rb);
				std::vector<uint8_t> pkg_buf(2200);
				uint16_t seq;
				if (auto* rel_ptr = pipereg.try_get<Reliability>(e)) {
					rel_ptr->sendData(seq, ByteSpan{buf});
					pkg_buf[0] = PKG_TYPE::RelData;
					const auto bw = tunnel_pkg_reldata_pack(buf.data(), buf.size(), 0x33, seq, pkg_buf.data()+1, pkg_buf.size()-1);
					assert(bw != 0);
					assert(bw == 4 + (size_t)rb);
					pkg_buf.resize(1 + bw);
				} else {
					assert(false && "impl unrel");
				}
				if (rng()%10 != 0) { // 10% drop chance
					pwq.queue.push_back(std::move(pkg_buf));
					std::printf("-> send data %hu\n", seq);
				} else {
					std::printf("-> dropped data %hu\n", seq);
				}
			}
		}

		// process incoming rel data
		for (auto&& [e, fds, rel] : pipereg.view<FDs, Reliability>().each()) {
			std::vector<uint8_t> rdata;
			while (rel.popData(rdata)) {
				const auto rb = write(fds.fd_write, rdata.data(), rdata.size());
				if (rb <= 0) {
					break;
				}
				assert((size_t)rb == rdata.size());
			}
		}

		// write test data
		while (i_outer < 90 && rng()%2) {
			static uint8_t counter{0};
			for (size_t i = 0; i < 1010; i++) {
				write(pipefd_alice_in[1], &counter, 1);
				counter_alice_in += 1;
				counter += 1;
			}
		}
		while (i_outer < 90 && rng()%2) {
			static uint8_t counter{0};
			for (size_t i = 0; i < 1010; i++) {
				write(pipefd_bob_in[1], &counter, 1);
				counter_bob_in += 1;
				counter += 1;
			}
		}

		// read and check test data
		while (true) {
			std::vector<uint8_t> buf(1200);
			const auto rb = read(pipefd_alice_out[0], buf.data(), buf.size());
			if (rb <= 0) {
				break; // nothing more to read or something failed
			}
			buf.resize(rb);
			std::printf("read %zd on alices side\n", rb);
			static uint8_t counter{0};
			for (const auto it : buf) {
				assert(it == counter);
				counter += 1;
			}
			counter_alice_out += buf.size();
		}
		while (true) {
			std::vector<uint8_t> buf(1200);
			const auto rb = read(pipefd_bob_out[0], buf.data(), buf.size());
			if (rb <= 0) {
				break; // nothing more to read or something failed
			}
			buf.resize(rb);
			std::printf("read %zd on bobs side\n", rb);
			static uint8_t counter{0};
			for (const auto it : buf) {
				assert(it == counter);
				counter += 1;
			}
			counter_bob_out += buf.size();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1 + rng()%3));
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	std::printf("counter_alice_in: %zu\n", counter_alice_in);
	std::printf("counter_alice_out: %zu\n", counter_alice_out);
	std::printf("counter_bob_in: %zu\n", counter_bob_in);
	std::printf("counter_bob_out: %zu\n", counter_bob_out);

	assert(counter_alice_in == counter_bob_out);
	assert(counter_bob_in == counter_alice_out);
	assert(pipereg.get<Reliability>(ent_alice).unacked_data.empty());
	assert(pipereg.get<Reliability>(ent_bob).unacked_data.empty());
}

int main(void) {
	test_pipe_raw();
	test_pipe_pkg();

	return 0;
}
