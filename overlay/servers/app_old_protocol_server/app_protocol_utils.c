#include "root_common_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void app_old_protocol_data_cryption(uint8_t key, uint8_t *dat, uint8_t len)
{
	uint8_t i = 0;
	for (i = 0; i < len; i++)
	{
		dat[i] ^= key;
	}
}

// RC4的密钥调度算法
void keySchedulingAlgorithm(unsigned char *key, int keyLength, unsigned char *S)
{
    int i, j = 0;
    for (i = 0; i < 256; i++)
    {
        S[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        j = (j + S[i] + key[i % keyLength]) % 256;
        unsigned char temp = S[i]; // change
        S[i] = S[j];
        S[j] = temp;
    }
}

/*
 *def: RC4的伪随机生成算法
 *
 */
void pseudoRandomGenerationAlgorithm(unsigned char *S, unsigned char *data, int dataLength)
{
    int i = 0, j = 0, k;
    for (int n = 0; n < dataLength; n++)
    {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;

        // 交换S[i]和S[j]
        unsigned char temp = S[i];
        S[i] = S[j];
        S[j] = temp;

        k = S[(S[i] + S[j]) % 256];
        data[n] ^= k; // 将生成的伪随机字节流与数据进行异或
    }
}

// RC4加密/解密函数
void rc4(unsigned char *key, unsigned int keyLength, unsigned char *data, unsigned int dataLength)
{
    if (!key || keyLength == 0 || !data || dataLength == 0)
        return;
    unsigned char S[256];
    keySchedulingAlgorithm(key, keyLength, S);
    pseudoRandomGenerationAlgorithm(S, data, dataLength);

    memset(S, 0, sizeof(S));
}
