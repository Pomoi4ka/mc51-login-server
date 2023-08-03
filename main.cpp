#include <memory>
#include <random>
#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>

#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

#define PROTO_VER 51

class Crypter {
private:
    EVP_CIPHER_CTX* enc;
    EVP_CIPHER_CTX* dec;
public:
    Crypter(std::vector<uint8_t> sharedSecret) {
        enc = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(enc);
        EVP_EncryptInit(enc, EVP_aes_128_cfb8(), sharedSecret.data(), sharedSecret.data());
        dec = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(dec);
        EVP_DecryptInit(dec, EVP_aes_128_cfb8(), sharedSecret.data(), sharedSecret.data());
    }

    ~Crypter() {
        EVP_CIPHER_CTX_free(enc);
        EVP_CIPHER_CTX_free(dec);
    }

    void encrypt(void *dst, const void* src, int inlen)
    {
        int outlen;
        if (EVP_EncryptUpdate(enc, (unsigned char*) dst, &outlen,
                              (const unsigned char*) src, inlen) <= 0) {
            throw "OpenSSL exception";
        }
    }

    void decrypt(void *dst, const void* src, int inlen)
    {
        int outlen;
        if (EVP_DecryptUpdate(dec, (unsigned char*) dst, &outlen,
                              (const unsigned char*) src, inlen) <= 0) {
            throw "OpenSSL exception";
        }
    }
};

class BufStream {
    std::unique_ptr<Crypter> crypter;
private:
    bool isClosed;
    int fd;
    std::vector<uint8_t> readBuffer;
    struct {
        size_t pos;
        size_t avail;
    } rB;
    std::vector<uint8_t> writeBuffer;
    struct {
        size_t pos;
        size_t avail;
    } wB;

    int fetch()
    {
        if (rB.pos >= readBuffer.size()) rB.pos = 0;
        size_t n = readBuffer.size() - rB.pos + rB.avail;

        int result = ::read(fd, &readBuffer[rB.pos + rB.avail], n);

        rB.avail += result;
        return result;
    }
public:
    BufStream(int fd) {
        isClosed = false;
        this->fd = fd;
        readBuffer.resize(4096);
        writeBuffer.resize(4096);
    }

    void setEncryption(std::vector<uint8_t> sharedSecret)
    {
        this->crypter = std::unique_ptr<Crypter>(new Crypter(sharedSecret));
    }

    bool isAbleToRead()
    {
        return rB.avail || !isStreamClosed();
    }

    bool isStreamClosed()
    {
        return isClosed;
    }

    bool flush(int all = false)
    {
        do {
            int result = ::write(fd, &writeBuffer[wB.pos - wB.avail], wB.avail);
            if (result < 0) {
                if (errno == EPIPE) isClosed = true;
                return false;
            }
            wB.avail -= result;
        } while (all && wB.avail);

        return true;
    }

    int write(const void* buf, size_t n)
    {
        ssize_t pos = 0;

        while ((size_t) pos < n) {
            if (wB.pos == writeBuffer.size()) wB.pos = 0;
            if (wB.avail == writeBuffer.size() &&
                !flush()) {
                return 0;
            }
            size_t avail = writeBuffer.size() - wB.avail;
            if (avail > n - pos) avail = n - pos;
            if (crypter) {
                crypter->encrypt(&writeBuffer[wB.pos], &((char*)buf)[pos], avail);
            } else {
                memcpy(&writeBuffer[wB.pos], &((char*)buf)[pos], avail);
            }
            wB.pos += avail;
            wB.avail += avail;
            pos += avail;
        }
        return 1;
    }

    ssize_t read(void* buf, size_t n)
    {
        ssize_t pos = 0;

        while ((size_t) pos < n) {
            if (!rB.avail && fetch() <= 0) {
                isClosed = true;
                return pos;
            }
            size_t cnt;
            if (n - pos > rB.avail) {
                cnt = rB.avail;
            } else {
                cnt = n - pos;
            }
            if (crypter) {
                crypter->decrypt((char*)buf + pos, &readBuffer[rB.pos], cnt);
            } else {
                memcpy((char*)buf + pos, &readBuffer[rB.pos], cnt);
            }
            pos += cnt;
            rB.pos += cnt;
            rB.avail -= cnt;
        }
        return pos;
    }

    template <typename T>
    T read()
    {
        T t;
        read(&t, sizeof(T));
        return t;
    }

    template <typename T>
    T readbe()
    {
        T v = read<T>();
        uint8_t* t = (uint8_t*)&v;
        for (size_t i = 0; i < sizeof(T)/2; ++i) {
            size_t oi = sizeof(T) - i - 1;
            uint8_t tmp = t[i];
            t[i] = t[oi];
            t[oi] = tmp;
        }
        return v;
    }

    template <typename T>
    void write(T t)
    {
        write(&t, sizeof(T));
    }

    template <typename T>
    void writebe(T t)
    {
        uint8_t *p = (uint8_t*)&t;
        for (size_t i = 0; i < sizeof(T)/2; ++i) {
             size_t oi = sizeof(T) - i - 1;
             uint8_t tmp = p[i];
             p[i] = p[oi];
             p[oi] = tmp;
        }
        write(t);
    }

    template <typename S, typename T>
    void writeV(std::vector<T> v, bool sizeInBytes = false, bool sendBigEndianed = true)
    {
        if (sizeInBytes)
            writebe<S>(v.size()*sizeof(v[0]));
        else
            writebe<S>(v.size());

        for (auto& i: v) {
             sendBigEndianed ?
                 writebe(i) : write(i);
        }
    }
};

template <>
std::string BufStream::read<std::string>()
{
    std::string s;
    auto s_len = readbe<uint16_t>();
    for (auto i = 0; i < s_len; ++i) {
        s.push_back(readbe<uint16_t>());
    }

    return s;
}

template <>
void BufStream::write<std::string>(std::string s)
{
    writebe<uint16_t>(s.size());
    for (const auto& i: s) {
        writebe<uint16_t>(i);
    }
}

// TODO: make login/register command
// TODO: introduce database capatibility with old authme
// TODO: make proxy connect to main server
// TODO: config for the program
// TODO: omit some packets which can lead to coodinate leaks
// TODO: user notification system and ability to get individual login history

class ClientHandler: private BufStream {
private:
    int sock;
    int peid;
    std::string name;
public:
    ClientHandler(int sock):
        BufStream(sock)
    {
        this->sock = sock;
    }

    int getPacketId()
    {
        return read<uint8_t>();
    }

    void sendStatus()
    {
        if (read<uint8_t>() != 1) {
            return;
        }

        std::vector<uint16_t> message;
        auto pushChar = [&message] (unsigned char val) {
            message.push_back(val);
        };

        pushChar(0xa7);
        pushChar('1');
        pushChar(0);

        auto pushStr = [&message, pushChar] (std::string s) {
            for (auto& c: s) pushChar(c);
            pushChar(0);
        };
        pushStr("" + PROTO_VER);
        pushStr("1.4.7");
        pushStr("Proxy says hello");
        pushStr("9999");
        pushStr("-1");
        message.resize(message.size() - 1);

        write<uint8_t>(0xff);  // Kick packet
        writeV<short>(message);
        flush();
    }

    bool expectPacket(int pid)
    {
        int apid = getPacketId();
        if (pid != apid) {
            fprintf(stderr, "channel %d: expected packet 0x%x but got 0x%x\n", sock, pid, apid);
            return false;
        }

        return true;
    }

    bool handshake()
    {
        if (read<uint8_t>() != PROTO_VER) return false;

        name = read<std::string>(); // username
        read<std::string>(); // host
        readbe<int>(); // port

        write<uint8_t>(0xfd);
        write<std::string>("-"); // server id

        BIGNUM* bn = BN_new();
        int bits = 1200;
        if (BN_set_word(bn, RSA_F4) != 1) {
            throw "Failed to set RSA exponent";
        }

        RSA *rsa = RSA_new();

        if (RSA_generate_key_ex(rsa, bits, bn, NULL) != 1) {
            RSA_free(rsa);
            throw "Could not generate RSA key";
        }

        unsigned char* rsaData = NULL;
        int rsaLength = i2d_RSA_PUBKEY(rsa, &rsaData);

        if (rsaLength < 0) {
            RSA_free(rsa);
            throw "Could not get RSA public key";
        }

        writebe<short>(rsaLength);
        write(rsaData, rsaLength);
        OPENSSL_free(rsaData);

        std::random_device r;
        uint32_t vToken = r();
        writebe<short>(sizeof(vToken));
        write<uint32_t>(vToken);
        flush();

        std::vector<uint8_t> sharedSecret;
        std::vector<uint8_t> verifyToken;
        std::vector<uint8_t> decryptedSharedSecret;

        if (!expectPacket(0xfc)) {
            goto ret;
        }
        sharedSecret.resize(readbe<uint16_t>());
        read(sharedSecret.data(), sharedSecret.size());
        verifyToken.resize(readbe<uint16_t>());
        read(verifyToken.data(), verifyToken.size());

        {
            std::vector<uint8_t> decryptedVToken;
            decryptedVToken.resize(RSA_size(rsa));
            int len = RSA_private_decrypt(verifyToken.size(), verifyToken.data(),
                                          decryptedVToken.data(), rsa, RSA_PKCS1_PADDING);

            if (len <= 0) {
                fprintf(stderr, "channel %d: could not decrypt verify token\n", sock);
                goto ret;
            }
            decryptedVToken.resize(len);

            if (decryptedVToken.size() != sizeof(vToken) ||
                *(uint32_t*)decryptedVToken.data() != vToken) {
                fprintf(stderr, "channel %d: verify token is incorrect\n", sock);
                goto ret;
            }
        }

        decryptedSharedSecret.resize(RSA_size(rsa));
        {
            int len = RSA_private_decrypt(sharedSecret.size(), sharedSecret.data(),
                                          decryptedSharedSecret.data(), rsa, RSA_PKCS1_PADDING);
            if (len <= 0) {
                fprintf(stderr, "channel %d: could not decrypt shared secret\n", sock);
                goto ret;
            }
            decryptedSharedSecret.resize(len);
            /* Encryption respnose */
            write<uint8_t>(0xfc);
            write<uint16_t>(0);
            write<uint16_t>(0);

            setEncryption(decryptedSharedSecret);
        }

        flush();
        if (!expectPacket(0xcd)) {
            goto ret;
        }
        read<uint8_t>();
        RSA_free(rsa);
        return true;
    ret:
        RSA_free(rsa);
        return false;
    }

    void handle_client()
    {
        write<uint8_t>(0x01); // login packet
        peid = ((unsigned) rand()) & 0xfffffffu;
        write<uint32_t>(peid);
        write<std::string>("default");
        write<uint8_t>(0); // gamemode
        write<uint8_t>(0); // dimension
        write<uint8_t>(3); // difficulty
        write<uint8_t>(0); // not used
        write<uint8_t>(20); // max players
        flush();

        write<uint8_t>(0xca); // player abilities
        write<uint8_t>(0); // flags
        write<uint8_t>(12); // fly speed
        write<uint8_t>(25); // walking speed

        write<uint8_t>(0x04); // time update
        write<long>(0);
        write<long>(0);

        write<uint8_t>(0x0d); // player pos and look
        writebe<double>(0);
        writebe<double>(0);
        writebe<double>(1.8);
        writebe<double>(0);
        writebe<float>(0);
        writebe<float>(0);
        write<uint8_t>(0);

        write<uint8_t>(0xc9); // list item
        write<std::string>(name);
        write<bool>(true);
        writebe<short>(50);

        write<uint8_t>(0x68); // set window items
        write<uint8_t>(0); // window id
        writebe<short>(45); // number of slots
        for (int i = 0; i < 45; ++i) {
            writebe<short>(-1);
        }

        // clearing the cursor

        write<uint8_t>(0x67); // set slot
        write<uint8_t>(-1);
        writebe<short>(-1);
        writebe<short>(-1);

        write<uint8_t>(0x67); // set slot
        write<uint8_t>(0);
        writebe<short>(0);
        writebe<short>(-1);


        write<uint8_t>(0x38); // map chunk bulk
        int n = 16;
        writebe<short>(n);
        writebe<int>(0);
        write<uint8_t>(1);
        for (int i = 0; i < n; ++i) {
            int x = (i % 4) - 2;
            int y = (i/4) - 2;
            writebe<int>(x);
            writebe<int>(y);
            writebe<short>(0);
            writebe<short>(0);
        }
        flush();
        sleep(3423);
    }

    static void run(int sock)
    {
        ClientHandler handler(sock);

        uint8_t packetId = handler.getPacketId();
        switch (packetId) {
        case 0xfe: {
            handler.sendStatus();
        } break;
        case 0x02: {
            printf("Handshake\n");
            if (handler.handshake()) {
                handler.handle_client();
            }
        } break;
        default:
            printf("Invalid initial packet 0x%x\n", packetId);
            break;
        }
        close(sock);
    }
};

int main(int argc, char* argv[])
{
    int port = 25565;

    auto nextArg = [&argc, &argv] () -> const char* {
        if (!argc--) return NULL;
        return *argv++;
    };
    const char* progname = nextArg();
    (void) progname;

    while (true) {
        const char* arg = nextArg();
        if (!arg) break;
        if (strcmp(arg, "-port") == 0) {
           const char* val = nextArg();
           if (!val) {
               fprintf(stderr, "ERROR: argument `%s` expects value\n", arg);
               return 1;
           }
           char* endp;
           port = strtol(val, &endp, 10);
           if (*endp != '\0') {
               fprintf(stderr, "ERROR: invalid value for argument `%s`: `%s`\n", arg, val);
               return 1;
           }
        } else {
            fprintf(stderr, "ERROR: unknown argument: `%s`\n", arg);
            return 1;
        }
    }

    auto checkErr = [] (int val, const char* msg) {
        if (val < 0) {
            fprintf(stderr, "ERROR: %s: %s\n", msg, strerror(errno));
            exit(1);
        }
    };

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    checkErr(sock, "could not create socket");

    int opt = 1;
    checkErr(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)),
             "setsockopt() failed");
    checkErr(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)),
             "setsockopt() failed");

    sockaddr_in addr;
    addr.sin_addr.s_addr = 0;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    checkErr(bind(sock, (sockaddr*)&addr, sizeof(addr)),
             "bind() failed");

    checkErr(listen(sock, 5), "listen() failed");

    while (true) {
        sockaddr_in caddr;
        int caddr_len = sizeof(caddr);
        int csock = accept(sock, (sockaddr*)&caddr, (socklen_t*)&caddr_len);
        if (csock < 0) {
            fprintf(stderr, "ERROR: could not accept(): %s\n",
                    strerror(errno));
        }
        std::thread(ClientHandler::run, csock).detach();
    }

    close(sock);
    return 0;
}
