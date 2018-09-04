#ifndef SECURE_CHANNEL_H
#define SECURE_CHANNEL_H

#define SECURE_CHANNEL_CIPHER_BLOCK_SIZE        16
#define SECURE_CHANNEL_CIPHER_BLOCK_HEX_SIZE    32
#define SECURE_CHANNEL_RANDOM_SIZE              16
#define SECURE_CHANNEL_RANDOM_HEX_SIZE          32
#define SECURE_CHANNEL_RANDOM_HALF_SIZE         8
#define SECURE_CHANNEL_HASH_VALUE_SIZE          32
#define SECURE_CHANNEL_HASH_VALUE_HEX_SIZE      64

//����˵��
//0)sc_key:                Լ������ͨ����Կ
//1)hex:                   16����
//2)����ı�random_text:    random1 + random2
//3)��Կkey:                sha256(random1 + sc_k + random2)
//4)����plian_text
//5)����cipher_text:        1bytes padding_count + cipher + hmac: sha256(random2 + plain_text + random1))

#endif

