#ifndef SECURE_CHANNEL_H
#define SECURE_CHANNEL_H

#define SECURE_CHANNEL_CIPHER_BLOCK_SIZE        16
#define SECURE_CHANNEL_CIPHER_BLOCK_HEX_SIZE    32
#define SECURE_CHANNEL_RANDOM_SIZE              16
#define SECURE_CHANNEL_RANDOM_HEX_SIZE          32
#define SECURE_CHANNEL_RANDOM_HALF_SIZE         8
#define SECURE_CHANNEL_HASH_VALUE_SIZE          32
#define SECURE_CHANNEL_HASH_VALUE_HEX_SIZE      64

//定义说明
//0)sc_key:                约定传输通道密钥
//1)hex:                   16进制
//2)随机文本random_text:    random1 + random2
//3)密钥key:                sha256(random1 + sc_k + random2)
//4)明文plian_text
//5)密文cipher_text:        1bytes padding_count + cipher + hmac: sha256(random2 + plain_text + random1))

#endif

