#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// [TunnelID u32] [type u8] [name_len u8] [name N bytes] [timeout u32] [reliable u8]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_info_pack(uint32_t tunnel_id, uint8_t type, const char* name, uint8_t name_len, uint32_t timeout, uint8_t reliable, uint8_t* buf, size_t buf_len);

// [TunnelID u32] [type u8] [name_len u8] [name N bytes] [timeout u32] [reliable u8]
// returns bytes consumed from buf on success, 0 on error
// name_len is without null byte
// name is the returned pointer into buf
size_t tunnel_pkg_info_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id, uint8_t* type, const uint8_t** name, uint8_t* name_len, uint32_t* timeout, uint8_t* reliable);

// [TunnelID u32]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_init_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len);

// [TunnelID u32]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_init_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id);

// [TunnelID u32]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_init_ack_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len);

// [TunnelID u32]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_init_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id);

// [TunnelID u32]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_ack_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len);

// [TunnelID u32]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id);

// [TunnelID u32]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_kill_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len);

// [TunnelID u32]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_kill_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id);

// --- Pipe Handshake ---

// [TunnelID u32]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_pipe_init_pack(uint32_t tunnel_id, uint8_t* buf, size_t buf_len);

// [TunnelID u32]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_pipe_init_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id);

// [TunnelID u32] [PipeID u16]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_pipe_init_ack_pack(uint32_t tunnel_id, uint16_t pipe_id, uint8_t* buf, size_t buf_len);

// [TunnelID u32] [PipeID u16]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_pipe_init_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id, uint16_t* pipe_id);

// [TunnelID u32] [PipeID u16]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_pipe_ack_pack(uint32_t tunnel_id, uint16_t pipe_id, uint8_t* buf, size_t buf_len);

// [TunnelID u32] [PipeID u16]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_pipe_ack_unpack(const uint8_t* buf, size_t buf_len, uint32_t* tunnel_id, uint16_t* pipe_id);

// [PipeID u16]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_pipe_kill_pack(uint16_t pipe_id, uint8_t* buf, size_t buf_len);

// [PipeID u16]
// returns bytes consumed from buf on success, 0 on error
size_t tunnel_pkg_pipe_kill_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id);

// --- Data Packets ---

// [PipeID u16] [data ...]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_data_pack(const uint8_t* data, size_t data_len, uint16_t pipe_id, uint8_t* buf, size_t buf_len);

// [PipeID u16] [data ...]
// returns bytes consumed from buf on success, 0 on error
// data is the returned pointer into buf
size_t tunnel_pkg_data_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id, const uint8_t** data, size_t* data_len);

// [PipeID u16] [seq u16] [data ...]
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_reldata_pack(const uint8_t* data, size_t data_len, uint16_t pipe_id, uint16_t seq, uint8_t* buf, size_t buf_len);

// [PipeID u16] [seq u16] [data ...]
// returns bytes consumed from buf on success, 0 on error
// data is the returned pointer into buf
size_t tunnel_pkg_reldata_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id, uint16_t* seq, const uint8_t** data, size_t* data_len);

// [PipeID u16] [seq0 u16] [seq1 u16] ...
// returns bytes written to buf on success, 0 on error
size_t tunnel_pkg_reldata_ack_pack(const uint16_t* seqs, size_t seq_count, uint16_t pipe_id, uint8_t* buf, size_t buf_len);

// [PipeID u16] [seq0 u16] [seq1 u16] ...
// returns bytes consumed from buf on success, 0 on error
// seqs needs to be an array with enough space
size_t tunnel_pkg_reldata_ack_unpack(const uint8_t* buf, size_t buf_len, uint16_t* pipe_id, uint16_t* seqs, size_t seqs_buf_len, size_t* seq_count);

#ifdef __cplusplus
} // extern c
#endif
