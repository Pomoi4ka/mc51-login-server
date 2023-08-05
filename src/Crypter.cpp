#include "Crypter.hpp"

Crypter::Crypter(std::vector<uint8_t> sharedSecret)
{
    enc = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(enc);
    EVP_EncryptInit(enc, EVP_aes_128_cfb8(), sharedSecret.data(), sharedSecret.data());
    dec = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(dec);
    EVP_DecryptInit(dec, EVP_aes_128_cfb8(), sharedSecret.data(), sharedSecret.data());
}

Crypter::~Crypter()
{
    EVP_CIPHER_CTX_free(enc);
    EVP_CIPHER_CTX_free(dec);
}

void Crypter::encrypt(void *dst, const void* src, int inlen)
{
    int outlen;
    if (EVP_EncryptUpdate(enc, (unsigned char*) dst, &outlen,
                          (const unsigned char*) src, inlen) <= 0) {
        throw "OpenSSL exception";
    }
}

void Crypter::decrypt(void *dst, const void* src, int inlen)
{
    int outlen;
    if (EVP_DecryptUpdate(dec, (unsigned char*) dst, &outlen,
                          (const unsigned char*) src, inlen) <= 0) {
        throw "OpenSSL exception";
    }
}
