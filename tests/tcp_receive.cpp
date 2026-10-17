// Host-side regression test for macOS cross-builds. Socket reads are scripted;
// the actual VehicleEvent.cpp and Compressor.cpp implementations are exercised.
// clang++ -std=c++20 -I include tests/tcp_receive.cpp src/Compressor.cpp -lz -o /tmp/beammp-tcp-tests
// /tmp/beammp-tcp-tests

#if !defined(__APPLE__)
#error "Run this host-side test with the native macOS C++ compiler"
#endif
#ifdef NDEBUG
#error "This regression test requires assertions to be enabled"
#endif

#include <arpa/inet.h>
#include <sys/socket.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

using SOCKET = uint64_t;
using SOCKADDR_IN = sockaddr_in;
using SOCKADDR = sockaddr;
int WSAGetLastError() { return 5; }
void WSACleanup() {}

struct Read { std::string bytes; int result; };
std::vector<Read> reads;
size_t nextRead = 0;
int closedSockets = 0;
int testRecv(int, char* buffer, int length, int flags) {
    assert(flags == MSG_WAITALL);
    assert(nextRead < reads.size());
    const auto& read = reads[nextRead++];
    if (read.result > 0) {
        assert(read.result <= length);
        assert(size_t(read.result) == read.bytes.size());
        std::memcpy(buffer, read.bytes.data(), read.bytes.size());
    }
    return read.result;
}

#define recv testRecv
#include "../src/Network/VehicleEvent.cpp"
#undef recv

bool Terminate = false;
std::string UlStatus;
int KillSocket(uint64_t) { ++closedSockets; return 0; }
void debug(const std::string&) {}
void error(const std::string&) {}
void info(const std::string&) {}
void GameSend(std::string_view) {}
void CoreSend(std::string) {}
void ServerParser(std::string_view) {}
void SyncResources(uint64_t) {}

void reset(std::vector<Read> incoming) {
    reads = std::move(incoming);
    nextRead = 0;
    Terminate = false;
    closedSockets = 0;
    UlStatus.clear();
}
std::string header(int32_t size) {
    return std::string(reinterpret_cast<const char*>(&size), sizeof(size));
}
void consumed() { assert(nextRead == reads.size()); }

int main() {
    char buffer[16]{};
    reset({{"ab", 2}, {"c", 1}, {"de", 2}});
    assert(RecvWaitAll(42, buffer, 5) == 5);
    assert(std::string(buffer, 5) == "abcde");
    consumed();

    reset({{"ab", 2}, {"", 0}});
    assert(RecvWaitAll(42, buffer, 5) == 2);
    assert(std::string(buffer, 2) == "ab");
    consumed();

    reset({{"", 0}});
    assert(RecvWaitAll(42, buffer, 5) == 0);
    consumed();

    reset({{"", -1}});
    assert(RecvWaitAll(42, buffer, 5) == -1);
    consumed();

    reset({{"ab", 2}, {"", -1}});
    assert(RecvWaitAll(42, buffer, 5) == -1);
    consumed();

    reset({});
    assert(RecvWaitAll(42, buffer, 0) == 0);
    consumed();

    const auto h = header(5);
    reset({{h.substr(0, 1), 1}, {h.substr(1), 3}, {"ab", 2}, {"cde", 3}});
    assert(TCPRcv(42) == "abcde");
    assert(!Terminate);
    consumed();

    reset({{h.substr(0, 2), 2}, {"", 0}});
    assert(TCPRcv(42).empty());
    assert(Terminate);
    consumed();

    reset({{h, 4}, {"ab", 2}, {"", 0}});
    assert(TCPRcv(42).empty());
    assert(Terminate);
    consumed();

    reset({{h, 4}, {"", -1}});
    assert(TCPRcv(42).empty());
    assert(Terminate && closedSockets == 1);
    consumed();

    reset({{header(0), 4}});
    assert(TCPRcv(42).empty());
    assert(Terminate);
    consumed();

    reset({});
    assert(TCPRcv(SOCKET(-1)).empty());
    assert(Terminate);
    consumed();

    std::string payload(700000, 'x');
    auto zipped = Comp(std::span<const char>(payload.data(), payload.size()));
    auto packet = "ABG:" + std::string(zipped.data(), zipped.size());
    reset({{header(packet.size()), 4}, {packet, int(packet.size())}});
    assert(TCPRcv(42) == payload);
    assert(!Terminate);
    consumed();

    packet = "ABG:bad-zlib";
    reset({{header(packet.size()), 4}, {packet, int(packet.size())}});
    assert(TCPRcv(42).empty());
    consumed();

    reset({{header(5), 4}, {"Etest", 5}});
    assert(TCPRcv(42) == "Etest");
    assert(UlStatus == "UlDisconnected: test");
    consumed();

    std::cout << "PASS: 15 TCP receive/decompression cases using actual VehicleEvent.cpp and Compressor.cpp\n";
}
