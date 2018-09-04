#ifndef SECURE_CHANNEL_C_H
#define SECURE_CHANNEL_C_H
#include "secure-channel.h"

#ifdef SECURE_CHANNEL_C_STATIC
#   define SECURE_CHANNEL_C_API
#else
#   if defined(_MSC_VER)
#       ifdef SECURE_CHANNEL_C_EXPORT
#           define SECURE_CHANNEL_C_API __declspec(dllexport)
#       else
#           define SECURE_CHANNEL_C_API __declspec(dllimport)
#       endif
#   else
#       define SECURE_CHANNEL_C_API __attribute__ ((visibility ("default")))
#   endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

//��ʼ��
SECURE_CHANNEL_C_API int secure_channel_initialize_client();

//����ʼ��
SECURE_CHANNEL_C_API int secure_channel_uninitialize_client();

//ȡ�ÿ�汾��
SECURE_CHANNEL_C_API int secure_channel_version_client();

//����
SECURE_CHANNEL_C_API int secure_channel_decrypt_client(
    const char     *cipher_text_hex,
    unsigned int    cipher_text_hex_len,
    unsigned char   random_hex[SECURE_CHANNEL_RANDOM_HEX_SIZE],
    unsigned char  *plain_text,
    unsigned int    plain_text_len);

#ifdef __cplusplus
}
#endif

#endif

