#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <arpa/inet.h>

class BufStream {
private:
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
        this->fd = fd;
        readBuffer.resize(4096);
        writeBuffer.resize(4096);
    }

    ssize_t read(void* buf, size_t n)
    {
        ssize_t pos = 0;

        while ((size_t) pos < n) {
            if (!rB.avail && fetch() <= 0) return pos;
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
};

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
        int packetId = 0;
        read(&packetId, 1);
        return packetId;
    }

    static void run(int sock)
    {
        ClientHandler handler(sock);

        printf("packetId: %d\n", handler.getPacketId());
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
