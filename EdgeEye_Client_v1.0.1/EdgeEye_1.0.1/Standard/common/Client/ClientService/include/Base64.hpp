#ifndef __BASE64_HPP__
#define __BASE64_HPP__

int base64_encode(unsigned char in[], unsigned char out[], int len, int newline_flag);
int base64_decode(unsigned char in[], unsigned char out[], int len);
#endif
