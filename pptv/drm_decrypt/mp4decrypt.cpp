#include <string.h>
#include "mp4decrypt.h"

void decrypt_sampledata(aes_ctx &ctx, unsigned char in[], unsigned char out[], unsigned int size)
{
    unsigned int block_count = size / 16;
    unsigned int remainder   = size - block_count * 16;

    // decrypt data block
    for (unsigned int i = 0; i < block_count; ++i)
    {
        aes_decrypt(&ctx, in, out);
        in  += 16;
        out += 16;
    }

    if (remainder > 0)
    {
        memcpy(out, in, remainder);
    }
}

