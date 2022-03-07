#ifndef CRYPTO_H
#define CRYPTO_H

#include <limits>
#include <string>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

static const unsigned int KEY_SIZE = 32;
static const unsigned int BLOCK_SIZE = 16;
static const unsigned int BUFFER_SIZE = 65;

template <typename T>
struct zallocator
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    pointer address (reference v) const {return &v;}
    const_pointer address (const_reference v) const {return &v;}

    pointer allocate (size_type n, const void* hint = 0) 
    {
        (void)(hint);
        if (n > std::numeric_limits<size_type>::max() / sizeof(T))
            throw std::bad_alloc();
        return static_cast<pointer> (::operator new (n * sizeof (value_type)));
    }

    void deallocate(pointer p, size_type n) 
    {
        OPENSSL_cleanse(p, n*sizeof(T));
        ::operator delete(p); 
    }
    
    size_type max_size() const 
    {
        return std::numeric_limits<size_type>::max() / sizeof (T);
    }
    
    template<typename U>
    struct rebind
    {
        typedef zallocator<U> other;
    };

    void construct (pointer ptr, const T& val)
    {
        new (static_cast<T*>(ptr) ) T (val);
    }

    void destroy(pointer ptr)
    {
        static_cast<T*>(ptr)->~T();
    }

#if __cpluplus >= 201103L
    template<typename U, typename... Args>
    void construct (U* ptr, Args&&  ... args)
    {
        ::new (static_cast<void*> (ptr) ) U (std::forward<Args> (args)...);
    }

    template<typename U>
    void destroy(U* ptr)
    {
        ptr->~U();
    }
#endif
};

typedef unsigned char byte;
typedef std::basic_string<char, std::char_traits<char>, zallocator<char> > secure_string;

class Crypto 
{
public:
	Crypto();
	~Crypto();

	unsigned char *rsa_encrypt(unsigned char *plaintext, int *len, int *fileLen);
	unsigned char *rsa_decrypt(unsigned char *ciphertext, int *len);
	int aes_encrypt(/*const byte key[KEY_SIZE], const byte iv[BLOCK_SIZE], */const secure_string& ptext, 
                 secure_string& ctext);
	int aes_decrypt(/*const byte key[KEY_SIZE], const byte iv[BLOCK_SIZE], */const secure_string& ctext, 
                 secure_string& rtext);
    
    int sha256(char *string, char outputBuffer[BUFFER_SIZE]);

private:
	void gen_params(byte key[KEY_SIZE], byte iv[BLOCK_SIZE]);
	RSA* createRSA(unsigned char * key, int isPublic);
	int getRsaPublicKey(EVP_PKEY **pubKey);
	int getRsaPrivateKey(EVP_PKEY **priKey);
};

#endif
