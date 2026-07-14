#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../src/tunnel_packets.h"

static void test_pkg_info_roundtrip(void) {
	uint32_t tunnel_id = 12345;
	uint8_t type = 0;
	const char* name = "hello";
	uint8_t name_len = 5;
	uint32_t timeout = 300;
	uint8_t reliable = 1;

	uint8_t buf[256];
	size_t packed_len = tunnel_pkg_info_pack(tunnel_id, type, name, name_len, timeout, reliable, buf, sizeof(buf));
	// 4 (tunnel_id) + 1 (type) + 1 (name_len) + 5 (name) + 4 (timeout) + 1 (reliable) = 16
	assert(packed_len == 16);

	uint32_t decoded_tunnel_id;
	uint8_t decoded_type;
	char decoded_name[257]; // +1 for our null
	uint8_t decoded_name_len;
	uint32_t decoded_timeout;
	uint8_t decoded_reliable;
	size_t unpacked_len = tunnel_pkg_info_unpack(buf, packed_len, &decoded_tunnel_id, &decoded_type, decoded_name, &decoded_name_len, &decoded_timeout, &decoded_reliable);
	assert(unpacked_len == packed_len);

	assert(decoded_tunnel_id == tunnel_id);
	assert(decoded_type == type);
	assert(decoded_name_len == name_len);
	decoded_name[decoded_name_len] = '\0';
	assert(strcmp(decoded_name, name) == 0);
	assert(decoded_timeout == timeout);
	assert(decoded_reliable == reliable);

	printf("test_pkg_info_roundtrip: PASSED\n");
}

static void test_pkg_info_datagram(void) {
	uint32_t tunnel_id = 99999;
	uint8_t type = 1;
	const char* name = "x";
	uint8_t name_len = 1;
	uint32_t timeout = 0;
	uint8_t reliable = 0;

	uint8_t buf[256];
	size_t packed_len = tunnel_pkg_info_pack(tunnel_id, type, name, name_len, timeout, reliable, buf, sizeof(buf));
	assert(packed_len == 12); // 4 + 1 + 1 + 1 + 4 + 1 = 12

	uint32_t decoded_tunnel_id;
	uint8_t decoded_type;
	char decoded_name[257];
	uint8_t decoded_name_len;
	uint32_t decoded_timeout;
	uint8_t decoded_reliable;
	size_t unpacked_len = tunnel_pkg_info_unpack(buf, packed_len, &decoded_tunnel_id, &decoded_type, decoded_name, &decoded_name_len, &decoded_timeout, &decoded_reliable);
	assert(unpacked_len == packed_len);

	assert(decoded_tunnel_id == tunnel_id);
	assert(decoded_type == type);
	assert(decoded_name_len == name_len);
	assert(decoded_timeout == timeout);
	assert(decoded_reliable == reliable);

	printf("test_pkg_info_datagram: PASSED\n");
}

static void test_pkg_info_buffer_too_small(void) {
	uint8_t buf[5];
	size_t packed_len = tunnel_pkg_info_pack(1, 0, "hello", 5, 300, 1, buf, sizeof(buf));
	assert(packed_len == 0);

	printf("test_pkg_info_buffer_too_small: PASSED\n");
}

static void test_pkg_info_unpack_buffer_too_small(void) {
	uint8_t buf[] = { // partial
		0x39, 0x30, 0x00, 0x00, // tunnel id
		0x00, // type
		0x01, // namelen
		'a',  // name
		0x00, 0x00, 0x00, 0x00, // timeout
		0x00, // reliable
	};
	uint32_t tunnel_id;
	uint8_t type;
	char name[257];
	uint8_t name_len;
	uint32_t timeout;
	uint8_t reliable;
	size_t unpacked_len = tunnel_pkg_info_unpack(buf, 5, &tunnel_id, &type, name, &name_len, &timeout, &reliable);
	assert(unpacked_len == 0);

	printf("test_pkg_info_unpack_buffer_too_small: PASSED\n");
}

static void test_pkg_info_unpack_null_pointer(void) {
	uint8_t buf[256];
	uint32_t tunnel_id;
	uint8_t type;
	char name[257];
	uint8_t name_len;
	uint32_t timeout;
	uint8_t reliable;
	size_t unpacked_len = tunnel_pkg_info_unpack(NULL, 100, &tunnel_id, &type, name, &name_len, &timeout, &reliable);
	assert(unpacked_len == 0);

	unpacked_len = tunnel_pkg_info_unpack(buf, 100, NULL, NULL, NULL, NULL, NULL, NULL);
	assert(unpacked_len == 0);

	printf("test_pkg_info_unpack_null_pointer: PASSED\n");
}

static void test_pkg_info_pack_null_pointer(void) {
	uint8_t buf[256];
	size_t packed_len = tunnel_pkg_info_pack(1, 0, NULL, 5, 300, 1, buf, sizeof(buf));
	assert(packed_len == 0);

	printf("test_pkg_info_pack_null_pointer: PASSED\n");
}

static void test_pkg_info_unpack_empty_name(void) {
	uint8_t buf[] = {
		0x39, 0x30, 0x00, 0x00, // tunnel id
		0x00, // type
		0x00, // namelen !!
		0x00, 0x00, 0x00, 0x00 // ...
	};

	uint32_t decoded_tunnel_id;
	uint8_t decoded_type;
	char decoded_name[257]; // +1 for our null
	uint8_t decoded_name_len;
	uint32_t decoded_timeout;
	uint8_t decoded_reliable;
	size_t unpacked_len = tunnel_pkg_info_unpack(buf, sizeof(buf), &decoded_tunnel_id, &decoded_type, decoded_name, &decoded_name_len, &decoded_timeout, &decoded_reliable);
	assert(unpacked_len == 0);

	printf("test_pkg_info_unpack_empty_name: PASSED\n");
}

static void test_pkg_info_pack_empty_name(void) {
	uint32_t tunnel_id = 12345;
	uint8_t type = 0;
	const char* name = "";
	uint8_t name_len = 0;
	uint32_t timeout = 300;
	uint8_t reliable = 1;

	uint8_t buf[256];
	size_t packed_len = tunnel_pkg_info_pack(tunnel_id, type, name, name_len, timeout, reliable, buf, sizeof(buf));
	assert(packed_len == 0);

	printf("test_pkg_info_pack_empty_name: PASSED\n");
}

// --- Tunnel Handshake Tests ---

static void test_tunnel_init_roundtrip(void) {
	uint32_t tunnel_id = 42;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_init_pack(tunnel_id, buf, sizeof(buf));
	assert(packed_len == 4);

	uint32_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_init_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 4);
	assert(unpacked_id == tunnel_id);

	printf("test_tunnel_init_roundtrip: PASSED\n");
}

static void test_tunnel_init_ack_roundtrip(void) {
	uint32_t tunnel_id = 99999;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_init_ack_pack(tunnel_id, buf, sizeof(buf));
	assert(packed_len == 4);

	uint32_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_init_ack_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 4);
	assert(unpacked_id == tunnel_id);

	printf("test_tunnel_init_ack_roundtrip: PASSED\n");
}

static void test_tunnel_ack_roundtrip(void) {
	uint32_t tunnel_id = 12345678;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_ack_pack(tunnel_id, buf, sizeof(buf));
	assert(packed_len == 4);

	uint32_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_ack_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 4);
	assert(unpacked_id == tunnel_id);

	printf("test_tunnel_ack_roundtrip: PASSED\n");
}

static void test_tunnel_kill_roundtrip(void) {
	uint32_t tunnel_id = 0;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_kill_pack(tunnel_id, buf, sizeof(buf));
	assert(packed_len == 4);

	uint32_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_kill_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 4);
	assert(unpacked_id == tunnel_id);

	printf("test_tunnel_kill_roundtrip: PASSED\n");
}

static void test_tunnel_handshake_buffer_too_small(void) {
	uint8_t buf[2];

	assert(tunnel_pkg_init_pack(1, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_init_ack_pack(1, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_ack_pack(1, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_kill_pack(1, buf, sizeof(buf)) == 0);

	uint32_t id;
	assert(tunnel_pkg_init_unpack(buf, sizeof(buf), &id) == 0);
	assert(tunnel_pkg_init_ack_unpack(buf, sizeof(buf), &id) == 0);
	assert(tunnel_pkg_ack_unpack(buf, sizeof(buf), &id) == 0);
	assert(tunnel_pkg_kill_unpack(buf, sizeof(buf), &id) == 0);

	printf("test_tunnel_handshake_buffer_too_small: PASSED\n");
}

static void test_tunnel_handshake_null_pointer(void) {
	assert(tunnel_pkg_init_pack(1, NULL, 100) == 0);
	assert(tunnel_pkg_init_unpack(NULL, 100, NULL) == 0);
	assert(tunnel_pkg_init_ack_pack(1, NULL, 100) == 0);
	assert(tunnel_pkg_init_ack_unpack(NULL, 100, NULL) == 0);
	assert(tunnel_pkg_ack_pack(1, NULL, 100) == 0);
	assert(tunnel_pkg_ack_unpack(NULL, 100, NULL) == 0);
	assert(tunnel_pkg_kill_pack(1, NULL, 100) == 0);
	assert(tunnel_pkg_kill_unpack(NULL, 100, NULL) == 0);

	printf("test_tunnel_handshake_null_pointer: PASSED\n");
}

// --- Pipe Handshake Tests ---

static void test_pipe_init_roundtrip(void) {
	uint32_t tunnel_id = 111;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_pipe_init_pack(tunnel_id, buf, sizeof(buf));
	assert(packed_len == 4);

	uint32_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_pipe_init_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 4);
	assert(unpacked_id == tunnel_id);

	printf("test_pipe_init_roundtrip: PASSED\n");
}

static void test_pipe_init_ack_roundtrip(void) {
	uint32_t tunnel_id = 222;
	uint16_t pipe_id = 333;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_pipe_init_ack_pack(tunnel_id, pipe_id, buf, sizeof(buf));
	assert(packed_len == 6);

	uint32_t unpacked_tunnel_id;
	uint16_t unpacked_pipe_id;
	size_t unpacked_len = tunnel_pkg_pipe_init_ack_unpack(buf, packed_len, &unpacked_tunnel_id, &unpacked_pipe_id);
	assert(unpacked_len == 6);
	assert(unpacked_tunnel_id == tunnel_id);
	assert(unpacked_pipe_id == pipe_id);

	printf("test_pipe_init_ack_roundtrip: PASSED\n");
}

static void test_pipe_ack_roundtrip(void) {
	uint32_t tunnel_id = 444;
	uint16_t pipe_id = 555;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_pipe_ack_pack(tunnel_id, pipe_id, buf, sizeof(buf));
	assert(packed_len == 6);

	uint32_t unpacked_tunnel_id;
	uint16_t unpacked_pipe_id;
	size_t unpacked_len = tunnel_pkg_pipe_ack_unpack(buf, packed_len, &unpacked_tunnel_id, &unpacked_pipe_id);
	assert(unpacked_len == 6);
	assert(unpacked_tunnel_id == tunnel_id);
	assert(unpacked_pipe_id == pipe_id);

	printf("test_pipe_ack_roundtrip: PASSED\n");
}

static void test_pipe_kill_roundtrip(void) {
	uint16_t pipe_id = 0;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_pipe_kill_pack(pipe_id, buf, sizeof(buf));
	assert(packed_len == 2);

	uint16_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_pipe_kill_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 2);
	assert(unpacked_id == pipe_id);

	printf("test_pipe_kill_roundtrip: PASSED\n");
}

static void test_pipe_handshake_buffer_too_small(void) {
	uint8_t buf[1];

	assert(tunnel_pkg_pipe_init_pack(1, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_pipe_init_ack_pack(1, 1, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_pipe_ack_pack(1, 1, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_pipe_kill_pack(1, buf, sizeof(buf)) == 0);

	uint32_t id;
	uint16_t pid;
	assert(tunnel_pkg_pipe_init_unpack(buf, sizeof(buf), &id) == 0);
	assert(tunnel_pkg_pipe_init_ack_unpack(buf, sizeof(buf), &id, &pid) == 0);
	assert(tunnel_pkg_pipe_ack_unpack(buf, sizeof(buf), &id, &pid) == 0);
	assert(tunnel_pkg_pipe_kill_unpack(buf, sizeof(buf), &pid) == 0);

	printf("test_pipe_handshake_buffer_too_small: PASSED\n");
}

static void test_pipe_handshake_null_pointer(void) {
	assert(tunnel_pkg_pipe_init_pack(1, NULL, 100) == 0);
	assert(tunnel_pkg_pipe_init_unpack(NULL, 100, NULL) == 0);
	assert(tunnel_pkg_pipe_init_ack_pack(1, 1, NULL, 100) == 0);
	assert(tunnel_pkg_pipe_init_ack_unpack(NULL, 100, NULL, NULL) == 0);
	assert(tunnel_pkg_pipe_ack_pack(1, 1, NULL, 100) == 0);
	assert(tunnel_pkg_pipe_ack_unpack(NULL, 100, NULL, NULL) == 0);
	assert(tunnel_pkg_pipe_kill_pack(1, NULL, 100) == 0);
	assert(tunnel_pkg_pipe_kill_unpack(NULL, 100, NULL) == 0);

	printf("test_pipe_handshake_null_pointer: PASSED\n");
}

// --- Data Packet Tests ---

static void test_data_pack_unpack(void) {
	uint16_t pipe_id = 42;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t buf[256];

	size_t packed_len = tunnel_pkg_data_pack(data, sizeof(data), pipe_id, buf, sizeof(buf));
	assert(packed_len == 6); // 2 (pipe_id) + 4 (data)

	uint16_t unpacked_pipe_id;
	const uint8_t* unpacked_data;
	size_t unpacked_data_len;
	size_t unpacked_len = tunnel_pkg_data_unpack(buf, packed_len, &unpacked_pipe_id, &unpacked_data, &unpacked_data_len);
	assert(unpacked_len == packed_len);
	assert(unpacked_pipe_id == pipe_id);
	assert(unpacked_data_len == sizeof(data));
	assert(memcmp(unpacked_data, data, sizeof(data)) == 0);

	printf("test_data_pack_unpack: PASSED\n");
}

static void test_data_empty(void) {
	uint16_t pipe_id = 99;
	uint8_t buf[256];

	size_t packed_len = tunnel_pkg_data_pack(NULL, 0, pipe_id, buf, sizeof(buf));
	assert(packed_len == 0);

	printf("test_data_empty: PASSED\n");
}

static void test_reldata_pack_unpack(void) {
	uint16_t pipe_id = 10;
	uint16_t seq = 12345;
	uint8_t data[] = {0xAA, 0xBB, 0xCC};
	uint8_t buf[256];

	size_t packed_len = tunnel_pkg_reldata_pack(data, sizeof(data), pipe_id, seq, buf, sizeof(buf));
	assert(packed_len == 7); // 2 (pipe_id) + 2 (seq) + 3 (data)

	uint16_t unpacked_pipe_id;
	uint16_t unpacked_seq;
	const uint8_t* unpacked_data;
	size_t unpacked_data_len;
	size_t unpacked_len = tunnel_pkg_reldata_unpack(buf, packed_len, &unpacked_pipe_id, &unpacked_seq, &unpacked_data, &unpacked_data_len);
	assert(unpacked_len == packed_len);
	assert(unpacked_pipe_id == pipe_id);
	assert(unpacked_seq == seq);
	assert(unpacked_data_len == sizeof(data));
	assert(memcmp(unpacked_data, data, sizeof(data)) == 0);

	printf("test_reldata_pack_unpack: PASSED\n");
}

static void test_reldata_ack_pack_unpack(void) {
	uint16_t pipe_id = 77;
	uint16_t seqs[] = {1, 2, 3, 4, 5};
	uint8_t buf[256];

	size_t packed_len = tunnel_pkg_reldata_ack_pack(seqs, 5, pipe_id, buf, sizeof(buf));
	assert(packed_len == 12); // 2 (pipe_id) + 5*2 (seqs)

	uint16_t unpacked_pipe_id;
	uint16_t unpacked_seqs[5];
	size_t unpacked_seq_count;
	size_t unpacked_len = tunnel_pkg_reldata_ack_unpack(buf, packed_len, &unpacked_pipe_id, unpacked_seqs, 5, &unpacked_seq_count);
	assert(unpacked_len == packed_len);
	assert(unpacked_pipe_id == pipe_id);
	assert(unpacked_seq_count == 5);
	assert(memcmp(unpacked_seqs, seqs, 5 * sizeof(uint16_t)) == 0);

	printf("test_reldata_ack_pack_unpack: PASSED\n");
}

static void test_data_buffer_too_small(void) {
	uint8_t data[] = {0x01, 0x02};
	uint16_t seqs[] = {1};
	uint8_t buf[1];

	assert(tunnel_pkg_data_pack(data, sizeof(data), 1, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_reldata_pack(data, sizeof(data), 1, 0, buf, sizeof(buf)) == 0);
	assert(tunnel_pkg_reldata_ack_pack(seqs, 1, 1, buf, sizeof(buf)) == 0);

	uint16_t pid;
	const uint8_t* d;
	size_t dl;
	uint16_t s_buf[1];
	size_t sc;
	assert(tunnel_pkg_data_unpack(buf, sizeof(buf), &pid, &d, &dl) == 0);
	assert(tunnel_pkg_reldata_unpack(buf, sizeof(buf), &pid, NULL, &d, &dl) == 0);
	assert(tunnel_pkg_reldata_ack_unpack(buf, sizeof(buf), &pid, s_buf, 1, &sc) == 0);

	printf("test_data_buffer_too_small: PASSED\n");
}

static void test_data_null_pointer(void) {
	assert(tunnel_pkg_data_pack(NULL, 0, 1, NULL, 100) == 0);
	assert(tunnel_pkg_data_unpack(NULL, 100, NULL, NULL, NULL) == 0);
	assert(tunnel_pkg_reldata_pack(NULL, 0, 1, 0, NULL, 100) == 0);
	assert(tunnel_pkg_reldata_unpack(NULL, 100, NULL, NULL, NULL, NULL) == 0);
	assert(tunnel_pkg_reldata_ack_pack(NULL, 0, 1, NULL, 100) == 0);
	assert(tunnel_pkg_reldata_ack_unpack(NULL, 100, NULL, NULL, 0, NULL) == 0);

	printf("test_data_null_pointer: PASSED\n");
}

// --- Edge Case Tests ---

static void test_tunnel_id_boundary(void) {
	uint32_t tunnel_id = 0xFFFFFFFF;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_init_pack(tunnel_id, buf, sizeof(buf));
	assert(packed_len == 4);

	uint32_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_init_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 4);
	assert(unpacked_id == tunnel_id);

	printf("test_tunnel_id_boundary: PASSED\n");
}

static void test_pipe_id_boundary(void) {
	uint16_t pipe_id = 0xFFFF;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_pipe_init_ack_pack(1, pipe_id, buf, sizeof(buf));
	assert(packed_len == 6);

	uint32_t unpacked_tunnel_id;
	uint16_t unpacked_pipe_id;
	size_t unpacked_len = tunnel_pkg_pipe_init_ack_unpack(buf, packed_len, &unpacked_tunnel_id, &unpacked_pipe_id);
	assert(unpacked_len == 6);
	assert(unpacked_pipe_id == pipe_id);

	printf("test_pipe_id_boundary: PASSED\n");
}

static void test_seq_boundary(void) {
	uint16_t seq = 0xFFFF;
	uint8_t data[] = {0xAA};
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_reldata_pack(data, sizeof(data), 1, seq, buf, sizeof(buf));
	assert(packed_len == 5);

	uint16_t unpacked_pipe_id;
	uint16_t unpacked_seq;
	const uint8_t* unpacked_data;
	size_t unpacked_data_len;
	size_t unpacked_len = tunnel_pkg_reldata_unpack(buf, packed_len, &unpacked_pipe_id, &unpacked_seq, &unpacked_data, &unpacked_data_len);
	assert(unpacked_len == packed_len);
	assert(unpacked_seq == seq);

	printf("test_seq_boundary: PASSED\n");
}

static void test_name_max_length(void) {
	char name[257];
	memset(name, 'A', 255);
	name[256] = '\0';

	uint8_t buf[256 + 100];
	size_t packed_len = tunnel_pkg_info_pack(1, 0, name, 255, 0, 0, buf, sizeof(buf));
	assert(packed_len == 4 + 1 + 1 + 255 + 4 + 1);

	uint32_t unpacked_tunnel_id;
	uint8_t unpacked_type;
	char unpacked_name[257];
	uint8_t unpacked_name_len;
	uint32_t unpacked_timeout;
	uint8_t unpacked_reliable;
	size_t unpacked_len = tunnel_pkg_info_unpack(buf, packed_len, &unpacked_tunnel_id, &unpacked_type, unpacked_name, &unpacked_name_len, &unpacked_timeout, &unpacked_reliable);
	assert(unpacked_len == packed_len);
	assert(unpacked_name_len == 255);
	assert(strlen(unpacked_name) == 255);
	assert(memcmp(unpacked_name, name, 255) == 0);

	printf("test_name_max_length: PASSED\n");
}

static void test_reldata_ack_multiple_seqs(void) {
	uint16_t seqs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	uint8_t buf[256];

	size_t packed_len = tunnel_pkg_reldata_ack_pack(seqs, 10, 1, buf, sizeof(buf));
	assert(packed_len == 2 + 10 * 2);

	uint16_t unpacked_pipe_id;
	uint16_t unpacked_seqs[10];
	size_t unpacked_seq_count;
	size_t unpacked_len = tunnel_pkg_reldata_ack_unpack(buf, packed_len, &unpacked_pipe_id, unpacked_seqs, 10, &unpacked_seq_count);
	assert(unpacked_len == packed_len);
	assert(unpacked_seq_count == 10);
	assert(memcmp(unpacked_seqs, seqs, 10 * sizeof(uint16_t)) == 0);

	printf("test_reldata_ack_multiple_seqs: PASSED\n");
}

static void test_data_large_payload(void) {
	uint8_t data[1000];
	memset(data, 0xBB, sizeof(data));
	uint8_t buf[2000];

	size_t packed_len = tunnel_pkg_data_pack(data, sizeof(data), 1, buf, sizeof(buf));
	assert(packed_len == 2 + sizeof(data));

	uint16_t unpacked_pipe_id;
	const uint8_t* unpacked_data;
	size_t unpacked_data_len;
	size_t unpacked_len = tunnel_pkg_data_unpack(buf, packed_len, &unpacked_pipe_id, &unpacked_data, &unpacked_data_len);
	assert(unpacked_len == packed_len);
	assert(unpacked_data_len == sizeof(data));
	assert(memcmp(unpacked_data, data, sizeof(data)) == 0);

	printf("test_data_large_payload: PASSED\n");
}

static void test_exact_buffer_size(void) {
	uint8_t data[] = {0x01, 0x02, 0x03};
	uint8_t buf[5]; // 2 (pipe_id) + 3 (data)

	size_t packed_len = tunnel_pkg_data_pack(data, sizeof(data), 1, buf, sizeof(buf));
	assert(packed_len == 5);

	uint16_t unpacked_pipe_id;
	const uint8_t* unpacked_data;
	size_t unpacked_data_len;
	size_t unpacked_len = tunnel_pkg_data_unpack(buf, packed_len, &unpacked_pipe_id, &unpacked_data, &unpacked_data_len);
	assert(unpacked_len == packed_len);
	assert(unpacked_data_len == sizeof(data));
	assert(memcmp(unpacked_data, data, sizeof(data)) == 0);

	printf("test_exact_buffer_size: PASSED\n");
}

static void test_tunnel_id_zero(void) {
	uint32_t tunnel_id = 0;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_init_pack(tunnel_id, buf, sizeof(buf));
	assert(packed_len == 4);

	uint32_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_init_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 4);
	assert(unpacked_id == tunnel_id);

	printf("test_tunnel_id_zero: PASSED\n");
}

static void test_pipe_id_zero(void) {
	uint16_t pipe_id = 0;
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_pipe_kill_pack(pipe_id, buf, sizeof(buf));
	assert(packed_len == 2);

	uint16_t unpacked_id;
	size_t unpacked_len = tunnel_pkg_pipe_kill_unpack(buf, packed_len, &unpacked_id);
	assert(unpacked_len == 2);
	assert(unpacked_id == pipe_id);

	printf("test_pipe_id_zero: PASSED\n");
}

static void test_seq_zero(void) {
	uint16_t seq = 0;
	uint8_t data[] = {0xAA};
	uint8_t buf[16];

	size_t packed_len = tunnel_pkg_reldata_pack(data, sizeof(data), 1, seq, buf, sizeof(buf));
	assert(packed_len == 5);

	uint16_t unpacked_pipe_id;
	uint16_t unpacked_seq;
	const uint8_t* unpacked_data;
	size_t unpacked_data_len;
	size_t unpacked_len = tunnel_pkg_reldata_unpack(buf, packed_len, &unpacked_pipe_id, &unpacked_seq, &unpacked_data, &unpacked_data_len);
	assert(unpacked_len == packed_len);
	assert(unpacked_seq == seq);

	printf("test_seq_zero: PASSED\n");
}

int main(void) {
	test_pkg_info_roundtrip();
	test_pkg_info_datagram();
	test_pkg_info_buffer_too_small();
	test_pkg_info_unpack_buffer_too_small();
	test_pkg_info_unpack_null_pointer();
	test_pkg_info_pack_null_pointer();
	test_pkg_info_unpack_empty_name();
	test_pkg_info_pack_empty_name();
	test_tunnel_init_roundtrip();
	test_tunnel_init_ack_roundtrip();
	test_tunnel_ack_roundtrip();
	test_tunnel_kill_roundtrip();
	test_tunnel_handshake_buffer_too_small();
	test_tunnel_handshake_null_pointer();
	test_pipe_init_roundtrip();
	test_pipe_init_ack_roundtrip();
	test_pipe_ack_roundtrip();
	test_pipe_kill_roundtrip();
	test_pipe_handshake_buffer_too_small();
	test_pipe_handshake_null_pointer();
	test_data_pack_unpack();
	test_data_empty();
	test_reldata_pack_unpack();
	test_reldata_ack_pack_unpack();
	test_data_buffer_too_small();
	test_data_null_pointer();
	test_tunnel_id_boundary();
	test_pipe_id_boundary();
	test_seq_boundary();
	test_name_max_length();
	test_reldata_ack_multiple_seqs();
	test_data_large_payload();
	test_exact_buffer_size();
	test_tunnel_id_zero();
	test_pipe_id_zero();
	test_seq_zero();

	printf("\nAll tests passed!\n");
	return 0;
}
