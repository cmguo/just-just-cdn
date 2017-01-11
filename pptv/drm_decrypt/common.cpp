#include "SL_ByteBuffer.h"
#include "SL_Crypto_Hex.h"
#include "SL_Crypto_SHA256.h"
//#include "SL_Crypto_OSRand.h"
#include "SL_Crypto_AES.h"
#include "common.h"

int secure_channel_decrypt(
    const char     *sc_key,
    unsigned int    sc_key_len,
    const char     *cipher_text_hex,
    unsigned int    cipher_text_hex_len,
    unsigned char   random_hex[SECURE_CHANNEL_RANDOM_HEX_SIZE],
    unsigned char  *plain_text,
    unsigned int    plain_text_len)
{
    unsigned int cipher_text_len = cipher_text_hex_len / 2;
    if (cipher_text_len < (SECURE_CHANNEL_HASH_VALUE_SIZE + SECURE_CHANNEL_CIPHER_BLOCK_SIZE + 1))
    {
        return -10;
    }
    if (plain_text_len < cipher_text_len - SECURE_CHANNEL_HASH_VALUE_SIZE - 1)
    {
        return -11;
    }

    SL_Crypto_Hex hex;
    hex.set_upper_case(false);

    char random[SECURE_CHANNEL_RANDOM_SIZE] = { 0 };
    hex.decode(random_hex, SECURE_CHANNEL_RANDOM_HEX_SIZE, (unsigned char *)random, SECURE_CHANNEL_RANDOM_SIZE);

    SL_ByteBuffer buf(256);
    SL_Crypto_SHA256 sha256;

    //生成key: sha256(random1 + sc_key + random2)
    buf.write(random, SECURE_CHANNEL_RANDOM_HALF_SIZE);
    buf.write(sc_key, sc_key_len);
    buf.write(random + SECURE_CHANNEL_RANDOM_HALF_SIZE, SECURE_CHANNEL_RANDOM_HALF_SIZE);
    unsigned char key[SECURE_CHANNEL_HASH_VALUE_SIZE] = { 0 };
    sha256.final((unsigned char *)buf.data(), buf.data_size(), key);

    SL_ByteBuffer cipher_text(cipher_text_len);
    cipher_text.data_end(cipher_text_len);
    hex.decode((unsigned char *)cipher_text_hex, cipher_text_hex_len, (unsigned char *)cipher_text.data(), cipher_text_len);
    char *hmac1 = cipher_text.data() + cipher_text_len - SECURE_CHANNEL_HASH_VALUE_SIZE;

    SL_Crypto_AES aes;
    aes.init((const unsigned char *)key, NULL, 0, sizeof(key));
    plain_text_len = aes.decrypt((unsigned char *)cipher_text.data(), cipher_text_len - SECURE_CHANNEL_HASH_VALUE_SIZE, plain_text, plain_text_len);

    //生成hmac: sha256(random2 + plain_text + random1)
    buf.reset();
    buf.write(random + SECURE_CHANNEL_RANDOM_HALF_SIZE, SECURE_CHANNEL_RANDOM_HALF_SIZE);
    buf.write((char *)plain_text, plain_text_len);
    buf.write(random, SECURE_CHANNEL_RANDOM_HALF_SIZE);
    char hmac2[SECURE_CHANNEL_HASH_VALUE_SIZE] = { 0 };
    sha256.init();
    sha256.final((unsigned char *)buf.data(), buf.data_size(), (unsigned char *)hmac2);

    if (sl_memcmp(hmac1, hmac2, SECURE_CHANNEL_HASH_VALUE_SIZE))
    {
        return -12;
    }

    return 0;
}

