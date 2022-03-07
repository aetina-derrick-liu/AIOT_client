#ifdef __linux__ 
#include <arpa/inet.h>
#elif _WIN32
#include <winsock2.h>
#ifndef __MINGW32__
#pragma comment( lib, "ws2_32.lib")
#endif
#endif
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>	// std::stringstream
#include <stdexcept>
#include "Crypto.hpp"


using namespace std;
using EVP_CIPHER_CTX_free_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

const unsigned char rsa_public_key[] =
"-----BEGIN PUBLIC KEY-----\n"\
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAovkGAm9Squ8p9dkcqPqR\n"\
"7uw6+surXXwJejkriiQqa5SHs4acv6iwz0vGskb1aBMkH/XFH+PiGa0xLDYDv3Zk\n"\
"DF/T+lfXizqWivdvHA8OuQzuAlbQKcMxiZkzHBsKLJh+c7NikF2+rctt18thr2vV\n"\
"ehGLlFGp9a1a5FKR2C1ZmxOFpXMO9WXoAUcbOg5biX0mDYe5eymdTy1p2WVRSzSK\n"\
"O8nL3HH/9142HoP/NFw+8dXg3QVR0QGHW8Vnp+oZ6oYIn9HZOCOyrs3hjzy/Bf2f\n"\
"G2l9GZ75hsXOtQv+mFCaSheORN0JM/4MLUuPf/OCvajf92tAZkemO/B/+YI2+EPc\n"\
"EQIDAQAB\n"\
"-----END PUBLIC KEY-----\n";
const unsigned char rsa_private_key[] =
"-----BEGIN PRIVATE KEY-----\n"\
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCi+QYCb1Kq7yn1\n"\
"2Ryo+pHu7Dr6y6tdfAl6OSuKJCprlIezhpy/qLDPS8ayRvVoEyQf9cUf4+IZrTEs\n"\
"NgO/dmQMX9P6V9eLOpaK928cDw65DO4CVtApwzGJmTMcGwosmH5zs2KQXb6ty23X\n"\
"y2Gva9V6EYuUUan1rVrkUpHYLVmbE4Wlcw71ZegBRxs6DluJfSYNh7l7KZ1PLWnZ\n"\
"ZVFLNIo7ycvccf/3XjYeg/80XD7x1eDdBVHRAYdbxWen6hnqhgif0dk4I7KuzeGP\n"\
"PL8F/Z8baX0ZnvmGxc61C/6YUJpKF45E3Qkz/gwtS49/84K9qN/3a0BmR6Y78H/5\n"\
"gjb4Q9wRAgMBAAECggEAGUFV+RKfUPX3Hq4ijt0luEzLZwog6D/MiDBL2wB3wmwm\n"\
"C2AU8m6F19n6VK5CYXp14q4HekWqSChasms8cXM7Vd+TzfvG30CZC7gEc+ixc2PZ\n"\
"jms9nvssthITpYtbLEYFcpCd+ZlNb0lTIFnYLYMcdeo6IyX0nKncp/1TtJhyhmFk\n"\
"wgD8goFCvZsPyLZytuNP7SmymHnAsqX2RbW0S2/xyaUUbijUYX5SAe9blmxsXqNC\n"\
"ehFYjPcIUR+SyLoDeqayiqmmG+q7utPT2nnEGCdhSymP6g0vvg80OUn5Ky7ShWpD\n"\
"KyJbM3l7cSLQP4N1Mq0o0XgWmTpJMKv9/MGXE1+MMQKBgQDSzcGEuJEtFPDvXQ8l\n"\
"oaW9FnUNguFVovFY35JzPO4FXWv47w31GPNvSnsrkdX7xEHrubLtCEXrknxKpn2r\n"\
"g8iZ/loeVdXK6wue2WJKAPjUzUfgmI2E41aRTCNtlE4C1mzFyndX1dHUfEVOkCmp\n"\
"fA8xxVCGY6/3O+iRTUrY0+/b3wKBgQDF6gAFXD6RNZeshjq2hv8QoOz38E7nrN+V\n"\
"rC/7KoBDFtu0ZVfhEV4VxPoRS9AZKCSD+fy5uR16tgfHmPJTD3g8FQLASgi/MV55\n"\
"mmM29ml64+NsnvxwOJWY8sjYRXJ3e9B6TUoB4e+/pCmb7MVk8uVQQmpg2IZ7ZyW1\n"\
"yZFPS8lGDwKBgA/7q8WNXOyHHvF3I9PdmrxofH2Gw+EDVrs/UFJOd1mOyMX6EhvY\n"\
"RXTVt9sI1qmdKsZdsRRJwUtoMuZSE0sy1b3fTCPqrCgThDEKGGq2AifYXK/JwUkn\n"\
"F4QyuqjvSq7qZRrmDABr5Nnwhiy7v3zrPBew9zcJTek38J07uAYSo6gPAoGAGNj8\n"\
"D7A5GroDxOFFVc01EL1jgu1kuql1mLD5hSsc109zXf6rPdKUYV9NlgZSphXoH7mv\n"\
"BRI8pT9dUq3dHph+m+x3X/TrjDWCivnbIvdk3szlrKl5QA2amo1AsgZmDKI61mAF\n"\
"vp7DHuhZYP4k+juUlrL/8BC/MZcNoPkhiTcWmfkCgYB8IHLCFAizep99MRqnPfpR\n"\
"c0OmZZLbpmOSkPqlkFXtvWiYB3lVt6YGALSCrv3lF8B4rj4gNBckEmY+q7F3bWPc\n"\
"XdrrOVvgv7zF1KziPkWY+pl/UhQ0ahwgtrfbKftgslk54ppAj5Uc0g4564NoGWgj\n"\
"LyRU0a9RJnKc+Gx2F6fzIA==\n"\
"-----END PRIVATE KEY-----\n";

#ifndef AES_128
/* A 256 bit key */
unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
#else
/* A 128 bit key */
unsigned char *key = (unsigned char *)"962012D09B8170D9";
#endif

/* A 128 bit IV */
// unsigned char *iv = (unsigned char *)"0123456789012345";
unsigned char *iv = (unsigned char *)"A60F20B3346B89B5";


void handleOpenSSLErrors(void)
{
	ERR_print_errors_fp(stderr);
	abort();
}
							 
Crypto::Crypto() 
{
#ifndef AES_128
	EVP_add_cipher(EVP_aes_256_cbc()); // Load the necessary cipher
#else
	cout << "This is AES_128" << endl;
	EVP_add_cipher(EVP_aes_128_cbc()); // Load the necessary cipher
#endif
}

Crypto::~Crypto() 
{
}

/*void Crypto::gen_params(byte key[KEY_SIZE], byte iv[BLOCK_SIZE])
{
    int rc = RAND_bytes(key, KEY_SIZE);
    if (rc != 1)
      throw std::runtime_error("RAND_bytes key failed");

    rc = RAND_bytes(iv, BLOCK_SIZE);
    if (rc != 1)
      throw std::runtime_error("RAND_bytes for iv failed");
}*/

RSA* Crypto::createRSA(unsigned char *key, int isPublic)
{
	RSA *rsa = NULL;
	BIO *keybio;
	
	keybio = BIO_new_mem_buf(key, -1);
	if (keybio == NULL) 
	{
		cout << "Failed to create key BIO." << endl;
		return NULL;
	}
	if (isPublic) 
	{
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	}
	else 
	{
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	}
	if (rsa == NULL) 
	{
		cout << "Failed to create RSA" << endl;
	}
	BIO_free(keybio);

	return rsa;
}

int Crypto::getRsaPublicKey(EVP_PKEY **pubKey)
{
	RSA *rsaPublicKey = createRSA((unsigned char *)rsa_public_key, 1);

	if (!EVP_PKEY_assign_RSA(*pubKey, rsaPublicKey)) 
	{
		cout << "EVP_PKEY assign RSA(public key): failed." << endl;
		return -1;
	}

	return 0;
}

int Crypto::getRsaPrivateKey(EVP_PKEY **priKey)
{
	RSA *rsaPrivateKey = createRSA((unsigned char *)rsa_private_key, 0);

	if (!EVP_PKEY_assign_RSA(*priKey, rsaPrivateKey))
	{
		cout << "EVP_PKEY assign_RSA(private key): failed." << endl;
		return -1;
	}

	return 0;
}

/*
 * Hybrid cryptosystem
 */
unsigned char* Crypto::rsa_encrypt(unsigned char *plaintext, int *len, int *fileLen)
{
#ifdef NEWER_OPENSSLLIB
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
#else
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
#endif
	static EVP_PKEY *pkey = NULL;
	unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 };
	unsigned char *encrypted_key = NULL;
	unsigned char *ciphertext = NULL;
	int encrypted_key_length = 0;
	uint32_t eklen_n = 0;
	int pos = 0;
	int cipher_len = 0;
	int size_header = 0;

	pkey = EVP_PKEY_new();
	getRsaPublicKey(&pkey);

	encrypted_key = (unsigned char *)malloc(EVP_PKEY_size(pkey));
	encrypted_key_length = EVP_PKEY_size(pkey);

#ifdef NEWER_OPENSSLLIB
	if (!EVP_SealInit(ctx, EVP_aes_256_cbc(), &encrypted_key, &encrypted_key_length, iv, &pkey, 1)) {
		cout << "EVP_SealInit: failed." << endl;
		goto out_free;
	}
#else
	if (!EVP_SealInit(&ctx, EVP_aes_256_cbc(), &encrypted_key, &encrypted_key_length, iv, &pkey, 1)) {
		cout << "EVP_SealInit: failed." << endl;
		goto out_free;
	}
#endif
	
	eklen_n = htonl(encrypted_key_length);

	size_header = sizeof(eklen_n) + encrypted_key_length + EVP_CIPHER_iv_length(EVP_aes_256_cbc());

	/* max ciphertext len, see man EVP_CIPHER */
#ifdef NEWER_OPENSSLLIB
	cipher_len = *len + EVP_CIPHER_CTX_block_size(ctx) - 1;
#else
	cipher_len = *len + EVP_CIPHER_CTX_block_size(&ctx) - 1;
#endif

	// header(contains iv, encreypted key and encreypted key length) + data
	*fileLen = size_header + cipher_len;
	ciphertext = (unsigned char *)malloc(size_header + cipher_len);

	/* First we write out the encrypted key length, then the encrypted key,
	 * then the iv (the IV length is fixed by the cipher we have chosen).
	 */
	memcpy(ciphertext + pos, &eklen_n, sizeof(eklen_n));
	pos += sizeof(eklen_n);

	memcpy(ciphertext + pos, encrypted_key, encrypted_key_length);
	pos += encrypted_key_length;

	memcpy(ciphertext + pos, iv, EVP_CIPHER_iv_length(EVP_aes_256_cbc()));
	pos += EVP_CIPHER_iv_length(EVP_aes_256_cbc());

	/* Now we process the plaintext data and write the encrypted data to the
	 * ciphertext. cipher_len is filled with the length of ciphertext
	 * generated, len is the size of plaintext in bytes
	 * Also we have our updated position, we can skip the header via
	 * ciphertext + pos */
#ifdef NEWER_OPENSSLLIB
	if (!EVP_SealUpdate(ctx, ciphertext + pos, &cipher_len, plaintext, *len)) {
		cout << "EVP_SealUpdate: failed." << endl;
		goto out_free;
	}
	/* update ciphertext with the final remaining bytes */
	if (!EVP_SealFinal(ctx, ciphertext + pos + cipher_len, &cipher_len)) {
		cout << "EVP_SealFinal: failed." << endl;
		goto out_free;
	}
#else
	if (!EVP_SealUpdate(&ctx, ciphertext + pos, &cipher_len, plaintext, *len)) {
		cout << "EVP_SealUpdate: failed." << endl;
		goto out_free;
	}
	/* update ciphertext with the final remaining bytes */
	if (!EVP_SealFinal(&ctx, ciphertext + pos + cipher_len, &cipher_len)) {
		cout << "EVP_SealFinal: failed." << endl;
		goto out_free;
	}
#endif

out_free:
	if (pkey) {
		EVP_PKEY_free(pkey);
		pkey = NULL;
	}
	if (encrypted_key) {
		free(encrypted_key);
		encrypted_key = NULL;
	}
#ifdef NEWER_OPENSSLLIB
	EVP_CIPHER_CTX_free(ctx);
#else
	EVP_CIPHER_CTX_cleanup(&ctx);
#endif

	return ciphertext;
}

/*
 * Hybrid cryptosystem
 */
unsigned char* Crypto::rsa_decrypt(unsigned char *ciphertext, int *len)
{
#ifdef NEWER_OPENSSLLIB
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
#else
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
#endif
	EVP_PKEY *pkey = NULL;
	unsigned char *encrypted_key = NULL;
	unsigned int encrypted_key_length;
	uint32_t eklen_n;
	unsigned char iv[EVP_MAX_IV_LENGTH];
	int total_len = 0;

	pkey = EVP_PKEY_new();
	getRsaPrivateKey(&pkey);
	
	encrypted_key = (unsigned char *)malloc(EVP_PKEY_size(pkey));

	// plaintext will always be equal to or lesser than length of ciphertext 
	int plaintext_len = *len;

	// the length of ciphertest is at most plaintext + ciphers block size.
	int ciphertext_len = plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_cbc());

	unsigned char *plaintext = (unsigned char *)malloc(ciphertext_len);

	/* First need to fetch the encrypted key length, encrypted key and IV */
	int pos = 0;
	memcpy(&eklen_n, ciphertext + pos, sizeof(eklen_n));
	pos += sizeof(eklen_n);

	encrypted_key_length = ntohl(eklen_n);

	memcpy(encrypted_key, ciphertext + pos, encrypted_key_length);
	pos += encrypted_key_length;

	memcpy(iv, ciphertext + pos, EVP_CIPHER_iv_length(EVP_aes_256_cbc()));
	pos += EVP_CIPHER_iv_length(EVP_aes_256_cbc());

	// Now we have our encrypted_key and the iv we can decrypt the reamining
	// data
#ifdef NEWER_OPENSSLLIB
	if (!EVP_OpenInit(ctx, EVP_aes_256_cbc(), encrypted_key, encrypted_key_length, iv, pkey)) {
		cout << "EVP_OpenInit: failed." << endl;
		goto out_free;
	}

	if (!EVP_OpenUpdate(ctx, plaintext, &plaintext_len, ciphertext + pos, ciphertext_len)) {
		cout << "EVP_OpenUpdate: failed." << endl;
		goto out_free;
	}

	total_len = plaintext_len;

	if (!EVP_OpenFinal(ctx, plaintext + total_len, &plaintext_len))
		cout << "EVP_OpenFinal warning: failed." << endl;
#else
	if (!EVP_OpenInit(&ctx, EVP_aes_256_cbc(), encrypted_key, encrypted_key_length, iv, pkey)) {
		cout << "EVP_OpenInit: failed." << endl;
		goto out_free;
	}

	if (!EVP_OpenUpdate(&ctx, plaintext, &plaintext_len, ciphertext + pos, ciphertext_len)) {
		cout << "EVP_OpenUpdate: failed." << endl;
		goto out_free;
	}

	total_len = plaintext_len;

	if (!EVP_OpenFinal(&ctx, plaintext + total_len, &plaintext_len))
		cout << "EVP_OpenFinal warning: failed." << endl;
#endif

out_free:
	if (pkey) {
		EVP_PKEY_free(pkey);
		pkey = NULL;
	}
	if (encrypted_key) {
		free(encrypted_key);
		encrypted_key = NULL;
	}

#ifdef NEWER_OPENSSLLIB
	EVP_CIPHER_CTX_free(ctx);
#else
	EVP_CIPHER_CTX_cleanup(&ctx);
#endif

	return  plaintext;
}

int Crypto::aes_encrypt(const secure_string& ptext, secure_string& ctext)
{
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
#ifndef AES_128
    int rc = EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL, key, iv);
#else
	int rc = EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_cbc(), NULL, key, iv);
#endif
    if (rc != 1) 
	{
		cout << "EVP_EncryptInit_ex failed" << endl;
		return -1;
	}

    // Recovered text expands upto BLOCK_SIZE
    ctext.resize(ptext.size()+BLOCK_SIZE);
    int out_len1 = (int)ctext.size();

    rc = EVP_EncryptUpdate(ctx.get(), (byte*)&ctext[0], &out_len1, (const byte*)&ptext[0], (int)ptext.size());
    if (rc != 1) 
	{
		cout << "EVP_EncryptUpdate failed" << endl;
		return -2;
	}
    int out_len2 = (int)ctext.size() - out_len1;
    rc = EVP_EncryptFinal_ex(ctx.get(), (byte*)&ctext[0]+out_len1, &out_len2);
    if (rc != 1) 
	{
		cout << "EVP_EncryptFinal_ex failed" << endl;
		return -3;
	}
    // Set cipher text size now that we know it
    ctext.resize(out_len1 + out_len2);
	return 0;
}

int Crypto::aes_decrypt(/*const byte key[KEY_SIZE], const byte iv[BLOCK_SIZE], */const secure_string& ctext, secure_string& rtext)
{
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
#ifndef AES_128
    int rc = EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL, key, iv);
#else
	int rc = EVP_DecryptInit_ex(ctx.get(), EVP_aes_128_cbc(), NULL, key, iv);
#endif
    if (rc != 1) 
	{
		cout << "EVP_DecryptInit_ex failed" << endl;
		return -1;
	}
    // Recovered text contracts upto BLOCK_SIZE
    rtext.resize(ctext.size());
    int out_len1 = (int)rtext.size();

    rc = EVP_DecryptUpdate(ctx.get(), (byte*)&rtext[0], &out_len1, (const byte*)&ctext[0], (int)ctext.size());
    if (rc != 1) 
	{
		cout << "EVP_DecryptUpdate failed" << endl;
		return -2;
	}
    int out_len2 = (int)rtext.size() - out_len1;
    rc = EVP_DecryptFinal_ex(ctx.get(), (byte*)&rtext[0]+out_len1, &out_len2);
    if (rc != 1)
	{
		cout << "EVP_DecryptFinal_ex failed" << endl;
		return -3;
	}

    // Set recovered text size now that we know it
    rtext.resize(out_len1 + out_len2);
	return 0;
}

int Crypto::sha256(char *string, char outputBuffer[BUFFER_SIZE]){
    unsigned char hash[SHA256_DIGEST_LENGTH]; 
    SHA256_CTX sha256; 
    SHA256_Init(&sha256); 
    SHA256_Update(&sha256, string, strlen(string)); 
    SHA256_Final(hash, &sha256); 
    int i = 0; 
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    { 
     sprintf(outputBuffer + (i * 2), "%02x", hash[i]); 
    } 
    outputBuffer[BUFFER_SIZE-1] = 0; 
	return 0;
}