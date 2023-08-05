#ifndef BUF_STREAM_HPP_
#define BUF_STREAM_HPP_

#include "Crypter.hpp"

#include <memory>
#include <vector>
#include <cstdlib>

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

    int fetch();
public:
    BufStream(int fd);

    void setEncryption(std::vector<uint8_t> sharedSecret);
    bool isAbleToRead();
    bool isStreamClosed();
    bool flush(int all = false);
    int write(const void* buf, size_t n);
    ssize_t read(void* buf, size_t n);

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

    template <typename S, typename T>
    std::vector<T> readV()
    {
        std::vector<T> v;
        S count = readbe<S>();

        for (auto i = 0; i < count; ++i) {
            v.push_back(readbe<T>());
        }

        return v;
    }
};

#endif // BUF_STREAM_HPP_
