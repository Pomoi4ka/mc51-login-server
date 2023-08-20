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
        peid = ((unsigned) rand()) & 0xfffffffu;
        Packets::PacketLogin
            (peid, "default", 0, 0, 3, 20)
            .send(*this);

        Packets::PacketPlayerAbilities(2, 0, 0).send(*this);
        Packets::PacketTimeUpdate(0, 0).send(*this);
        Packets::PacketPlayerPositionAndLook
            (0, 60, 61.8, 0, 0, 0, 0)
            .send(*this);
        Packets::PacketPlayerListItem(name, true, 3).send(*this);
        Packets::PacketSetWindowItems(0, 45, std::vector<Slot>(45, Slot())).send(*this);
        Packets::PacketSetSlot(-1, -1, Slot()).send(*this);
        Packets::PacketSetSlot(0, 0, Slot()).send(*this);

        std::vector<Packets::ChunkMetaInformation> records;
        for (int i = 0; i < 16; ++i) {
            records.push_back({(i % 4) - 2,
                              (i/4) - 2,
                              0, 0});
        }

        Packets::PacketMapChunkBulk
            (16, 0, 1, std::vector<uint8_t>(), records)
            .send(*this);
        Packets::PacketChatMessage(L"\xa7\x65To register write \xa7l/reg <password> <confirmPassword>").send(*this);
        Packets::PacketChatMessage(L"Я сос биба").send(*this);

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
