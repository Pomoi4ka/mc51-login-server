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
#include <poll.h>

#include "BufStream.hpp"
#include "DataClasses.hpp"
#include <Packets.hpp>

#define PROTO_VER 51

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
        Packets::PacketHandshake h(*this);

        if (h.protocolVer != PROTO_VER) return 0;
        name = h.username;
        
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
        std::random_device r;
        uint32_t vToken = r();

        Packets::PacketEncryptionKeyRequest("-", std::vector<uint8_t>(rsaData, rsaData + rsaLength),
                                            std::vector<uint8_t>((char*) &vToken,
                                                                 ((char*)&vToken) + sizeof(vToken)))
            .send(*this);

        OPENSSL_free(rsaData);

        if (!expectPacket(0xfc)) {
            goto ret;
        }

        {
        std::vector<uint8_t> decryptedSharedSecret;
        std::vector<uint8_t> decryptedVToken;
        
        Packets::PacketEncryptionKeyResponse re(*this);
        decryptedVToken.resize(RSA_size(rsa));
        int len = RSA_private_decrypt(re.verifyToken.size(), re.verifyToken.data(),
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

        decryptedSharedSecret.resize(RSA_size(rsa));
        len = RSA_private_decrypt(re.sharedSecret.size(), re.sharedSecret.data(),
                                  decryptedSharedSecret.data(), rsa, RSA_PKCS1_PADDING);
        if (len <= 0) {
            fprintf(stderr, "channel %d: could not decrypt shared secret\n", sock);
            goto ret;
        }
        decryptedSharedSecret.resize(len);

        Packets::PacketEncryptionKeyResponse({}, {}).send(*this);
        
        
        setEncryption(decryptedSharedSecret);

        if (!expectPacket(0xcd)) {
            goto ret;
        }
        Packets::PacketClientStatuses cs(*this);
        RSA_free(rsa);
        return true;
        }
    ret:
        RSA_free(rsa);
        return false;
    }

    void waitForPacket(int packetId)
    {
        
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
        write<uint8_t>(2); // flags
        write<uint8_t>(0); // fly speed
        write<uint8_t>(0); // walking speed

        write<uint8_t>(0x04); // time update
        write<long>(0);
        write<long>(0);

        write<uint8_t>(0x0d); // player pos and look
        writebe<double>(0);
        writebe<double>(2000000);
        writebe<double>(2000001.8);
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

        write<uint8_t>(0x03); // chat message
        write<std::wstring>(L"\xa7\x65To register write \xa7l/reg <password> <confirmPassword>");
        flush();

        write<uint8_t>(0x03); // chat message
        write<std::wstring>(L"Я сос биба");
        flush();
        
        sleep(50);
        waitForPacket(0x03);
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
