#ifndef CRYPTER_HPP_
#define CRYPTER_HPP_

#include <vector>

#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

class Crypter {
private:
    EVP_CIPHER_CTX* enc;
    EVP_CIPHER_CTX* dec;
public:
    Crypter(std::vector<uint8_t> sharedSecret);
    ~Crypter();
    void encrypt(void *dst, const void* src, int inlen);
    void decrypt(void *dst, const void* src, int inlen);
};

#endif // CRYPTER_HPP_
