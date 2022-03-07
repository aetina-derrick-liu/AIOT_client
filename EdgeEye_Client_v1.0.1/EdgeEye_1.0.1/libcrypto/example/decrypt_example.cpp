#include <cstring> 
#include <fstream>      // std::ifstream, std::ofstream
#include <iostream>
#include "Crypto.hpp"

using std::string;

#define BUFFER_SIZE 64

void printBytesAsHex(unsigned char *bytes, size_t length, const char *message) {
	printf("%s: ", message);

	for (unsigned int i = 0; i < length; i++) {
#ifdef WIN32
		__mingw_printf("%02hhx", bytes[i]);
#else
		printf("%02hhx", bytes[i]);
#endif
	}

	puts("");
}

int main() 
{
	Crypto crypto;
	
	unsigned char str[BUFFER_SIZE] = "27#1629868908#3000#1517817311";
	unsigned char *ciphertext = NULL;
	unsigned char *plaintext = NULL;
	int len = BUFFER_SIZE;

	
	std::ifstream infile("chipher.dat", std::ifstream::binary);
	// get size of file
	infile.seekg(0, infile.end);
	long size = infile.tellg();
	infile.seekg(0);
	ciphertext = new unsigned char[size];
	infile.read(reinterpret_cast<char *>(ciphertext), size);

	printf("Begin to decrypt..\n");
	plaintext = crypto.rsa_decrypt(ciphertext, &len);

	printf("Decrypted string = {%s}\n", plaintext);

	if (strncmp((const char *)plaintext, (const char *)str, len))
		printf("Failed for the plaintext: {%s}\n", str);
	else
		printf("Ok, Decrypted string = {%s}\n", plaintext);

	if (ciphertext) {
		delete[] ciphertext;
		ciphertext = NULL;
	}
	if (plaintext) {
		free(plaintext);
		plaintext = NULL;
	}

	return 0;
}
