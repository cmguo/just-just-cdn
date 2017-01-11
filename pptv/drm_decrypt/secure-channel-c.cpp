#include "SL_Crypto_Hex.h"
//#include "SL_Crypto_CRTRand.h"
#include "SL_Crypto_SHA256.h"
#include "SL_ByteBuffer.h"
#include "common.h"
#include "secure-channel-c.h"

#define SECURE_CHANNEL_C_VERSION    1
#define SECURE_CHANNEL_SC_KEY       "OWu%!vE#6oZcWseJ*H8LJrL*o4zUMeF$"

int secure_channel_initialize_client()
{
    return 0;
}

int secure_channel_uninitialize_client()
{
    return 0;
}

int secure_channel_version_client()
{
    return SECURE_CHANNEL_C_VERSION;
}

int secure_channel_decrypt_client(
    const char     *cipher_text_hex,
    unsigned int    cipher_text_hex_len,
    unsigned char   random_hex[SECURE_CHANNEL_RANDOM_HEX_SIZE],
    unsigned char  *plain_text,
    unsigned int    plain_text_len)
{
    return secure_channel_decrypt(SECURE_CHANNEL_SC_KEY, sizeof(SECURE_CHANNEL_SC_KEY)-1,
        cipher_text_hex, cipher_text_hex_len, random_hex, plain_text, plain_text_len);
}

