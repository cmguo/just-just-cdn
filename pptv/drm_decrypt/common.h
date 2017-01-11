#ifndef SECURE_CHANNEL_COMMON_H
#define SECURE_CHANNEL_COMMON_H
#include "secure-channel.h"

int secure_channel_decrypt(
    const char     *sc_key,
    unsigned int    sc_key_len,
    const char     *cipher_text_hex,
    unsigned int    cipher_text_hex_len,
    unsigned char   random_hex[SECURE_CHANNEL_RANDOM_HEX_SIZE],
    unsigned char  *plain_text,
    unsigned int    plain_text_len);

#endif

