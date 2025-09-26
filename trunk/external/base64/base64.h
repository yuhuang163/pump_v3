/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef _BASE64_H__
#define _BASE64_H__

char * sec_base64_encode(const void *src, size_t len, size_t *out_len);
unsigned char * sec_base64_decode(const char *src, size_t len, size_t *out_len);
char * sec_base64_url_encode(const void *src, size_t len, size_t *out_len);
unsigned char * sec_base64_url_decode(const char *src, size_t len, size_t *out_len);

#endif /* BASE64_H */
