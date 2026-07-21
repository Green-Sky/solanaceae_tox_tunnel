#include "tunnel_packets.h"

#include <string.h>

static inline void write_u8(uint8_t* buf, uint8_t value) {
	buf[0] = value;
}

static inline void write_u16(uint8_t* buf, uint16_t value) {
	buf[0] = (value >> 0) & 0xFF;
	buf[1] = (value >> 8) & 0xFF;
}

static inline void write_u32(uint8_t* buf, uint32_t value) {
	buf[0] = (value >> 0) & 0xFF;
	buf[1] = (value >> 8) & 0xFF;
	buf[2] = (value >> 16) & 0xFF;
	buf[3] = (value >> 24) & 0xFF;
}

static inline uint8_t read_u8(const uint8_t* buf) {
	return buf[0];
}

static inline uint16_t read_u16(const uint8_t* buf) {
	return
		((uint16_t)buf[0] << 0) |
		((uint16_t)buf[1] << 8)
	;
}

static inline uint32_t read_u32(const uint8_t* buf) {
	return
		((uint32_t)buf[0] << 0) |
		((uint32_t)buf[1] << 8) |
		((uint32_t)buf[2] << 16) |
		((uint32_t)buf[3] << 24)
	;
}

size_t tunnel_pkg_info_pack(uint32_t tunnel_id, uint8_t type, const char* name, uint8_t name_len, uint32_t timeout, uint8_t reliable, uint8_t* buf, size_t buf_len) {
	if (!buf || !name || name_len == 0) return 0;

	size_t offset = 0;

	// TunnelID (u32)
	if (offset + 4 > buf_len) return 0;
	write_u32(buf + offset, tunnel_id);
	offset += 4;

	// type (u8)
	if (offset + 1 > buf_len) return 0;
	write_u8(buf + offset, type);
	offset += 1;

	// name_len (u8)
	if (offset + 1 > buf_len) return 0;
	write_u8(buf + offset, name_len);
	offset += 1;

	// name (N bytes)
	if (offset + name_len > buf_len) return 0;
	if (name_len > 0) {
		memcpy(buf + offset, name, name_len);
	}
	offset += name_len;

	// timeout (u32)
	if (offset + 4 > buf_len) return 0;
	write_u32(buf + offset, timeout);
	offset += 4;

	// reliable (u8)
	if (offset + 1 > buf_len) return 0;
	write_u8(buf + offset, reliable);
	offset += 1;

	return offset;
}

size_t tunnel_pkg_info_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id, uint8_t* type, const uint8_t** name, uint8_t* name_len, uint32_t* timeout, uint8_t* reliable) {
	if (!buf || !tunnel_id || !type || !name || !name_len || !timeout || !reliable) return 0;

	size_t offset = 0;

	// TunnelID (u32)
	if (offset + 4 > buf_len) return 0;
	*tunnel_id = read_u32(buf + offset);
	offset += 4;

	// type (u8)
	if (offset + 1 > buf_len) return 0;
	*type = read_u8(buf + offset);
	offset += 1;

	// name_len (u8)
	if (offset + 1 > buf_len) return 0;
	*name_len = read_u8(buf + offset);
	offset += 1;

	// needs to be at least 1
	if (*name_len == 0) return 0;

	// name (N bytes)
	if (offset + *name_len > buf_len) return 0;
	*name = buf + offset;
	offset += *name_len;

	// timeout (u32)
	if (offset + 4 > buf_len) return 0;
	*timeout = read_u32(buf + offset);
	offset += 4;

	// reliable (u8)
	if (offset + 1 > buf_len) return 0;
	*reliable = read_u8(buf + offset);
	offset += 1;

	return offset;
}

// --- InitTunnel (TunnelID) ---

size_t tunnel_pkg_init_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 4) return 0;
	write_u32(buf, tunnel_id);
	return 4;
}

size_t tunnel_pkg_init_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id) {
	if (!buf || !tunnel_id) return 0;

	if (buf_len < 4) return 0;
	*tunnel_id = read_u32(buf);
	return 4;
}

// --- InitAckTunnel (TunnelID) ---

size_t tunnel_pkg_init_ack_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 4) return 0;
	write_u32(buf, tunnel_id);
	return 4;
}

size_t tunnel_pkg_init_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id) {
	if (!buf || !tunnel_id) return 0;

	if (buf_len < 4) return 0;
	*tunnel_id = read_u32(buf);
	return 4;
}

// --- AckTunnel (TunnelID) ---

size_t tunnel_pkg_ack_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 4) return 0;
	write_u32(buf, tunnel_id);
	return 4;
}

size_t tunnel_pkg_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id) {
	if (!buf || !tunnel_id) return 0;

	if (buf_len < 4) return 0;
	*tunnel_id = read_u32(buf);
	return 4;
}

// --- KillTunnel (TunnelID) ---

size_t tunnel_pkg_kill_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 4) return 0;
	write_u32(buf, tunnel_id);
	return 4;
}

size_t tunnel_pkg_kill_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id) {
	if (!buf || !tunnel_id) return 0;

	if (buf_len < 4) return 0;
	*tunnel_id = read_u32(buf);
	return 4;
}

// --- InitPipe (TunnelID) ---

size_t tunnel_pkg_pipe_init_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 4) return 0;
	write_u32(buf, tunnel_id);
	return 4;
}

size_t tunnel_pkg_pipe_init_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id) {
	if (!buf || !tunnel_id) return 0;

	if (buf_len < 4) return 0;
	*tunnel_id = read_u32(buf);
	return 4;
}

// --- InitAckPipe (TunnelID + PipeID) ---

size_t tunnel_pkg_pipe_init_ack_pack(uint32_t tunnel_id, uint16_t pipe_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 6) return 0;
	write_u32(buf, tunnel_id);
	write_u16(buf + 4, pipe_id);
	return 6;
}

size_t tunnel_pkg_pipe_init_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id, uint16_t* pipe_id) {
	if (!buf || !tunnel_id || !pipe_id) return 0;

	if (buf_len < 6) return 0;
	*tunnel_id = read_u32(buf);
	*pipe_id = read_u16(buf + 4);
	return 6;
}

// --- AckPipe (TunnelID + PipeID) ---

size_t tunnel_pkg_pipe_ack_pack(uint32_t tunnel_id, uint16_t pipe_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 6) return 0;
	write_u32(buf, tunnel_id);
	write_u16(buf + 4, pipe_id);
	return 6;
}

size_t tunnel_pkg_pipe_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id, uint16_t* pipe_id) {
	if (!buf || !tunnel_id || !pipe_id) return 0;

	if (buf_len < 6) return 0;
	*tunnel_id = read_u32(buf);
	*pipe_id = read_u16(buf + 4);
	return 6;
}

// --- KillPipe (PipeID) ---

size_t tunnel_pkg_pipe_kill_pack(uint16_t pipe_id, uint8_t* buf, size_t buf_len) {
	if (!buf) return 0;

	if (buf_len < 2) return 0;
	write_u16(buf, pipe_id);
	return 2;
}

size_t tunnel_pkg_pipe_kill_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id) {
	if (!buf || !pipe_id) return 0;

	if (buf_len < 2) return 0;
	*pipe_id = read_u16(buf);
	return 2;
}

// --- Data (unreliable) ---

size_t tunnel_pkg_data_pack(const uint8_t* data, size_t data_len, uint16_t pipe_id, uint8_t* buf, size_t buf_len) {
	if (!buf || !data || data_len == 0) return 0;

	size_t header_len = 2;
	if (buf_len < header_len + data_len) return 0;
	write_u16(buf, pipe_id);
	memcpy(buf + header_len, data, data_len);
	return header_len + data_len;
}

size_t tunnel_pkg_data_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id, const uint8_t** data, size_t* data_len) {
	if (!buf || !pipe_id || !data || !data_len) return 0;

	if (buf_len < 2) return 0;
	*pipe_id = read_u16(buf);
	*data = buf + 2;
	*data_len = buf_len - 2;
	return buf_len;
}

// --- RelData (reliable) ---

size_t tunnel_pkg_reldata_pack(const uint8_t* data, size_t data_len, uint16_t pipe_id, uint16_t seq, uint8_t* buf, size_t buf_len) {
	if (!buf || !data || data_len == 0) return 0;

	size_t header_len = 4;
	if (buf_len < header_len + data_len) return 0;
	write_u16(buf, pipe_id);
	write_u16(buf + 2, seq);
	memcpy(buf + header_len, data, data_len);
	return header_len + data_len;
}

size_t tunnel_pkg_reldata_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id, uint16_t* seq, const uint8_t** data, size_t* data_len) {
	if (!buf || !pipe_id || !seq || !data || !data_len) return 0;

	if (buf_len < 4) return 0;
	*pipe_id = read_u16(buf);
	*seq = read_u16(buf + 2);
	*data = buf + 4;
	*data_len = buf_len - 4;
	return buf_len;
}

// --- RelDataAck ---

size_t tunnel_pkg_reldata_ack_pack(const uint16_t* seqs, size_t seq_count, uint16_t pipe_id, uint8_t* buf, size_t buf_len) {
	if (!buf || !seqs) return 0;

	size_t header_len = 2;
	size_t seqs_len = seq_count * 2;
	if (buf_len < header_len + seqs_len) return 0;
	write_u16(buf, pipe_id);
	if (seq_count > 0) {
		for (size_t i = 0; i < seq_count; i++) {
			write_u16(buf + header_len + i * 2, seqs[i]);
		}
	}
	return header_len + seqs_len;
}

size_t tunnel_pkg_reldata_ack_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id, uint16_t* seqs, size_t seqs_buf_len, size_t* seq_count) {
	if (!buf || !pipe_id || !seq_count) return 0;

	if (buf_len < 2) return 0;
	*pipe_id = read_u16(buf);
	*seq_count = (buf_len - 2) / 2;

	if (*seq_count > 0) {
		if (!seqs || seqs_buf_len < (size_t)*seq_count) return 0;
		for (size_t i = 0; i < (size_t)*seq_count; i++) {
			seqs[i] = read_u16(buf + 2 + i * 2);
		}
	}
	return buf_len;
}
