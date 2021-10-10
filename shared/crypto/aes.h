#pragma once

#include <vector>
#include <stdint.h>
#include <intrin.h>

#define BLOCK_SIZE 16

using aes_array = std::vector<uint8_t>;

class aes256
{
private:
	uint8_t m_key[32];
	uint8_t m_rkey[32];

	unsigned char m_buffer[3 * BLOCK_SIZE];
	unsigned char m_buffer_pos;

	size_t m_remainingLength;

	bool m_decryptInitialized;

	void check_and_encrypt_buffer(aes_array& encrypted);
	void check_and_decrypt_buffer(aes_array& plain);

	void encrypt(unsigned char* buffer);
	void decrypt(unsigned char* buffer);

	void expand_enc_key(unsigned char* rc);
	void expand_dec_key(unsigned char* rc);

	void sub_bytes(unsigned char* buffer);
	void sub_bytes_inv(unsigned char* buffer);

	void copy_key();

	void add_round_key(unsigned char* buffer, const unsigned char round);

	void shift_rows(unsigned char* buffer);
	void shift_rows_inv(unsigned char* buffer);

	void mix_columns(unsigned char* buffer);
	void mix_columns_inv(unsigned char* buffer);

public:

	aes256(uint8_t* key);
	~aes256();

	static size_t encrypt(uint8_t* key, void* plain, const size_t plain_length, aes_array& encrypted);
	static size_t encrypt(const aes_array& key, void* plain, const size_t plain_length, aes_array& encrypted);
	static size_t decrypt(uint8_t* key, void* encrypted, const size_t encrypted_length, aes_array& plain);
	static size_t decrypt(const aes_array& key, void* encrypted, const size_t encrypted_length, aes_array& plain);

	void encrypt_start(const size_t plain_length, aes_array& encrypted);
	void encrypt_continue(const unsigned char* plain, const size_t plain_length, aes_array& encrypted);
	void encrypt_end(aes_array& encrypted);

	void decrypt_start(const size_t encrypted_length);
	void decrypt_continue(const unsigned char* encrypted, const size_t encrypted_length, aes_array& plain);
};