#include "BufStream.hpp"
#include <cerrno>
#include <unistd.h>
#include <string>

BufStreamException::BufStreamException(Status status)
{
    this->status = status;
}

std::string BufStreamException::getError() const
{
    switch (status) {
    case STATUS_CONNECTION_CLOSED:
        return "Connection Closed";
    case STATUS_TIMEOUT:
        return "Timed out";
    }
    return "Unreachable";
}

BufStreamException::Status BufStreamException::getStatus() const
{
    return status;
}

std::ostream& operator<<(std::ostream& os, const BufStreamException& ex)
{
    os << ex.getError();
    return os;
}

template<>
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
        writebe<uint16_t>((unsigned char) i);
    }
}

template <>
void BufStream::write<std::wstring>(std::wstring s)
{
    writebe<uint16_t>(s.size());
    for (const auto& i: s) {
        writebe<uint16_t>((uint16_t) i);
    }
}

int BufStream::write(const void* buf, size_t n)
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

size_t BufStream::read(void* buf, size_t n)
{
    size_t pos = 0;

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

int BufStream::fetch()
{
    if (rB.pos >= readBuffer.size()) rB.pos = 0;
    size_t n = readBuffer.size() - rB.pos + rB.avail;

    int result = ::read(fd, &readBuffer[rB.pos + rB.avail], n);

    rB.avail += result;
    return result;
}

BufStream::BufStream(int fd)
{
    isClosed = false;
    this->fd = fd;
    readBuffer.resize(4096);
    writeBuffer.resize(4096);
    wB.avail = wB.pos = rB.avail = rB.pos = 0;
}

void BufStream::setEncryption(std::vector<uint8_t> sharedSecret)
{
    this->crypter = std::unique_ptr<Crypter>(new Crypter(sharedSecret));
}

bool BufStream::isAbleToRead()
{
    return rB.avail || !isStreamClosed();
}

bool BufStream::isStreamClosed()
{
    return isClosed;
}

bool BufStream::flush(int all)
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
