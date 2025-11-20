#ifndef __MBEDTLS_TEST_H__
#define __MBEDTLS_TEST_H__

int mbedtls_ctr_drbg_test(int verbose);
int mbedtls_entropy_test(int verbose);

int mbedtls_aes_test_1(int verbose);
int mbedtls_aes_test_2(int verbose);

int mbedtls_gcm_test_1(int verbose);
int mbedtls_gcm_test_2(int verbose);

int mbedtls_x509_crt_test(int verbose);

int mbedtls_md5_test(int verbose);

int mbedtls_sha1_test_1(int verbose);
int mbedtls_sha1_test_2(int verbose);

int mbedtls_sha256_test_1(int verbose);
int mbedtls_sha256_test_2(int verbose);

int mbedtls_sha512_test_1(int verbose);
int mbedtls_sha512_test_2(int verbose);

int mbedtls_hmac_test(int verbose);

int mbedtls_base64_test(int verbose);

int mbedtls_ccm_test(int verbose);

int mbedtls_rsa_test(int verbose);

int mbedtls_cmac_test(int verbose);

int mbedtls_pkcs5_test(int verbose);

int mbedtls_hmac_drbg_test(int verbose);

int mbedtls_ecjpake_test(int verbose);

#endif
