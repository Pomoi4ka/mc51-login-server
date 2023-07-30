#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <arpa/inet.h>

#define PROTO_VER 51

class BufStream {
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

    bool isStreamClosed()
    {
        return isClosed;
    }

    bool flush(int all = false)
    {
        do {
            int result = ::write(fd, &writeBuffer[wB.pos - wB.avail], wB.avail);
            if (result < 0) {
                if (result == EPIPE) isClosed = true;
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
            memcpy(&writeBuffer[wB.pos], &((char*)buf)[pos], avail);
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
            memcpy((char*)buf + pos, &readBuffer[rB.pos], cnt);
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

class ClientHandler: private BufStream {
private:
    int sock;
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

    void handshake()
    {
        if (read<uint8_t>() != PROTO_VER) return;

        auto name = read<std::string>(); // username
        read<std::string>(); // host
        readbe<int>(); // port

        printf("%s trying to login\n", name.c_str());
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
            handler.handshake();
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
