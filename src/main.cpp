#include <memory>
#include <random>
#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <poll.h>

#include <openssl/rand.h>

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

class ServerTunnel: private BufStream {
private:
    BufStream& client;
    std::string playerName;
    Packets::PacketLogin lp;
public:
    int reservedEID;
    int actualEID;

    ServerTunnel(int serverSock, int reservedEID, BufStream& client, std::string name):
        BufStream(serverSock),
        client(client),
        playerName(name),
        lp(0, "", 0, 0, 0, 0)
    {
        this->reservedEID = reservedEID;
    }

    void respawn()
    {
        Packets::PacketRespawn((int8_t)lp.dimension, lp.difficulty, lp.gamemode, 256, lp.levelType).send(client);
    }

    Packets::Packet* login()
    {
        Packets::PacketHandshake(PROTO_VER, playerName, "localhost", // may be serverHost and serverPort should be not hardcoded
                                 25565).send(*this);
        if (read<uint8_t>() != Packets::PacketEncryptionKeyRequest::PACKET_ID) {
            goto pidne;
        }
        {
        Packets::PacketEncryptionKeyRequest er(*this);

        const unsigned char* pk = er.publicKey.data();
        RSA* rsa = d2i_RSA_PUBKEY(NULL, &pk, er.publicKey.size());

        std::vector<uint8_t> ss(16);
        RAND_bytes(ss.data(), ss.size());
        std::vector<uint8_t> ess(RSA_size(rsa));
        RSA_public_encrypt(ss.size(), ss.data(), ess.data(), rsa, RSA_PKCS1_PADDING);
        std::vector<uint8_t> evk(RSA_size(rsa));
        RSA_public_encrypt(er.verifyToken.size(),
                           er.verifyToken.data(),
                           evk.data(), rsa, RSA_PKCS1_PADDING);
        Packets::PacketEncryptionKeyResponse(ess, evk).send(*this);

        if (read<uint8_t>() != Packets::PacketEncryptionKeyResponse::PACKET_ID) {
            goto pidne;
        }

        Packets::PacketEncryptionKeyResponse kr(*this);
        if (kr.sharedSecret.size() != 0 ||
            kr.verifyToken.size() != 0)
            throw "Encryption error";
        setEncryption(ss);
        }
        Packets::PacketClientStatuses(0).send(*this);
        if (read<uint8_t>() != Packets::PacketLogin::PACKET_ID) {
            goto pidne;
        }

        this->lp = Packets::PacketLogin(*this);

        this->actualEID = lp.entityID;
        return nullptr;
    pidne:

        return Packets::getPacket(*this);
    }

    void proxyAll()
    {
        client.flush();
        this->flush();

        enum Indices {
            SERVER = 0,
            CLIENT,
        };

        BufStream* streams[2];
        streams[SERVER] = this;
        streams[CLIENT] = &client;

        pollfd pfds[2];
        pfds[SERVER] = {this->getFd(), POLLIN, 0};
        pfds[CLIENT] = {client.getFd(), POLLIN, 0};

        Packets::Packet *p[2];
        while (true) {
            poll(pfds, sizeof(pfds)/sizeof(*pfds), -1);
            for (auto i = 0; i < sizeof(pfds)/sizeof(*pfds); ++i) {
                if (pfds[i].revents & POLLIN) {
                    p[i] = Packets::getPacket(*streams[i]);
                    if (i == SERVER)
                        p[i]->substituteEntityID(actualEID, reservedEID);
                    else
                        p[i]->substituteEntityID(reservedEID, actualEID);
                    printf("%s (%s)\n", Packets::PacketID2Cstr(p[i]->getID()),
                           i ? "client" : "server");
                    pfds[!i].events |= POLLOUT;
                    pfds[i].events &= ~POLLIN;
                }
                if (pfds[i].revents & POLLOUT) {
                    pfds[i].events &= ~POLLOUT;
                    pfds[!i].events |= POLLIN;
                    p[!i]->send(*streams[i]);
                    delete p[!i];
                }
                pfds[i].revents = 0;
            }
        }
    }
};

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
        pushStr("51");
        pushStr("1.4.7");
        pushStr("Proxy says hello");
        pushStr("9999");
        pushStr("666");
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

    template <class T>
    T* waitForPacket()
    {
        try {
        while (true) {
            Packets::Packet* p = Packets::getPacket(*this);
            if (p->getID() == T::PACKET_ID) {
                return (T*) p;
            }
            delete p;
        }
        } catch (BufStreamException e) {
            return nullptr;
        }
    }

    void handle_client(int addr, int port)
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

#define PROXY_PREFIX L"\xa7\x35[PROXY]\xa7r"

        auto* m = waitForPacket<Packets::PacketChatMessage>();
        if (m == nullptr) return;
        // TODO: register/login things + db stuff and etc.
        delete m;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);
        saddr.sin_addr.s_addr = addr;
        try {
        while (connect(sock, (sockaddr*)&saddr, sizeof(saddr)) < 0) {
            Packets::PacketChatMessage(PROXY_PREFIX L" Sorry the main server is not available yet :(").send(*this);
            for (int i = 5; i > 0; --i) {
                wchar_t msg[512];
                swprintf(msg, sizeof(msg)/sizeof(*msg), PROXY_PREFIX L" Trying to reconnect in %d seconds...", i);
                Packets::PacketChatMessage(msg).send(*this);
                sleep(1);
            }
        }
        Packets::PacketChatMessage(PROXY_PREFIX L" Successfully connected...").send(*this);

        ServerTunnel st(sock, peid, *this, name);
        Packets::Packet* p = nullptr;
        const char* ms = nullptr;
        try{
            p = st.login();
        } catch (const char* m) {
            ms = m;
        }
        if (p || ms) {
            char msg[1024];
            if (p && p->getID() == Packets::PacketDisconnect::PACKET_ID) {
                sprintf(msg, "[PROXY] Login failed: %s", ((Packets::PacketDisconnect*) p)->reason.c_str());
            } else if (ms) sprintf(msg, "[PROXY] Login failed: %s", ms);
            else {
                sprintf(msg, "[PROXY] Login failed");
                if (p) printf("%d\n", p->getID());
            }

            Packets::PacketDisconnect(msg).send(*this);
            if (p) delete p;
            return;
        }
        st.respawn();
        st.proxyAll();
        } catch (BufStreamException) {}
        close(sock);
    }

    static void run(int sock, int addr, int port)
    {
        ClientHandler handler(sock);
        signal(SIGPIPE, SIG_IGN);

        try {
        uint8_t packetId = handler.getPacketId();
        switch (packetId) {
        case 0xfe: {
            handler.sendStatus();
        } break;
        case 0x02: {
            printf("Handshake\n");
            if (handler.handshake()) {
                handler.handle_client(addr, port);
            }
        } break;
        default:
            printf("Invalid initial packet 0x%x\n", packetId);
            break;
        }
        } catch (BufStreamException e) {
            /* nothing to do */
        }
        close(sock);
    }
};

int main(int argc, char* argv[])
{
    int port = 25565;
    struct {
        int address;
        int port;
    } server;

    auto nextArg = [&argc, &argv] () -> const char* {
        if (!argc--) return NULL;
        return *argv++;
    };
    const char* progname = nextArg();
    (void) progname;

    while (true) {
        const char* arg = nextArg();
        auto missing_value = [&arg]() {
            fprintf(stderr, "ERROR: argument `%s` expects value\n", arg);
            exit(1);
        };

        if (!arg) break;
        if (strcmp(arg, "-port") == 0) {
           const char* val = nextArg();
           if (!val) missing_value();
           char* endp;
           port = strtol(val, &endp, 10);
           if (*endp != '\0') {
               fprintf(stderr, "ERROR: invalid value for argument `%s`: `%s`\n", arg, val);
               return 1;
           }
        } else if (strcmp(arg, "-server") == 0) {
            const char* val = nextArg();
            if (!val) missing_value();
            char address[256] = {0};

            const char* portStart = strchr(val, ':');
            if (portStart == nullptr) {
                fprintf(stderr, "ERROR: port is required\n");
                exit(1);
            }
            size_t addrLen = portStart - val;
            if (addrLen > sizeof(address)) {
                fprintf(stderr, "ERROR: address couldn't be so big: %s\n", arg);
            }

            memcpy(address, val, addrLen);
            portStart++;
            if (inet_pton(AF_INET, address, &server.address) == 0) {
                fprintf(stderr, "ERROR: invalid address: %s\n", address);
                exit(1);
            }
            char* endp;
            server.port = strtol(portStart, &endp, 10);
            if (*endp != '\0' || server.port < 0 || server.port > 65535) {
                fprintf(stderr, "ERROR: invalid port: %s\n", portStart);
                exit(1);
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
        std::thread(ClientHandler::run, csock, server.address, server.port).detach();
    }

    close(sock);
    return 0;
}
