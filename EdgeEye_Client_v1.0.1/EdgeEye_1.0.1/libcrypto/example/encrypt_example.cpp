#include <cstring>
#include <fstream>	// std::ifstream, std::ofstream
#include <iostream>
#include <string>
#include <unistd.h>
#include "Crypto.hpp"

using namespace std;

#define BUFFER_SIZE 64

void printBytesAsHex(unsigned char *bytes, size_t length, const char *message)
{
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

int main() {
	Crypto crypto;
	
	/* RSA */
	unsigned char text[] = "27#1629868908#3000#1517817311";
	unsigned char plaintext[BUFFER_SIZE] = { 0 };
	unsigned char *ciphertext = NULL;
	int len = BUFFER_SIZE;
	int fileLen = 0;

	memcpy(&plaintext, &text, sizeof(text));
	plaintext[sizeof(text)] = '\0';

	printf("String to be encrypted = {%s}\n", plaintext);
	printf("Begin to encrypt...\n");

	ciphertext = crypto.rsa_encrypt(plaintext, &len, &fileLen);

	if ((ciphertext!= NULL) && (fileLen != 0))
	{
		std::ofstream outfile("chipher.dat", std::ofstream::binary);
		outfile.write(reinterpret_cast<char *>(ciphertext), fileLen);
		outfile.close();
		printf("generate cipher file seccessfully\n");
	} else
	{
		printf("generate cipher file failed\n");
	}

	if (ciphertext)
	{
		free(ciphertext);
		ciphertext = NULL;
	}

	/* AES */
	secure_string ptext = "This is an example.";
	secure_string ctext, rtext;
	crypto.aes_encrypt(ptext, ctext);
	unsigned char test[ctext.length()];
   	copy(ctext.begin(), ctext.end(), test);
	printBytesAsHex(test, sizeof(test), "TEST");
    crypto.aes_decrypt(ctext, rtext);
	cout << "ptext:" << ptext << endl;
	cout << "rtext:" << rtext << endl;

	/* SHA256 */
	char szSHA256[BUFFER_SIZE] = {0};
	crypto.sha256("This is an example.", szSHA256);
	cout << "sha256_encrypt:" << szSHA256 << endl;

	return 0;
}
