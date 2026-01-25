#pragma once

#include <stdint.h>

class MicroInstruction {
public:
	MicroInstruction(uint64_t encoding) : encoding_(encoding) {}

    uint64_t GetEncoding() {
        return encoding_;
    }

    uint8_t GetReadSignals() const { return encoding_ & 0xF; }
    bool GetMemFetch() const { return (encoding_ >> 4) & 0x1; }
    bool GetMemRead() const { return (encoding_ >> 5) & 0x1; }
    bool GetMemWrite() const { return (encoding_ >> 6) & 0x1; }
    bool GetMarWrite() const { return (encoding_ >> 7) & 0x1; }
    bool GetMdrWrite() const { return (encoding_ >> 8) & 0x1; }
    bool GetPcWrite() const { return (encoding_ >> 9) & 0x1; } 
    bool GetSpWrite() const { return (encoding_ >> 10) & 0x1; }
    bool GetLvWrite() const { return (encoding_ >> 11) & 0x1; }
    bool GetCppWrite() const { return (encoding_ >> 12) & 0x1; }
    bool GetTosWrite() const { return (encoding_ >> 13) & 0x1; }
    bool GetOpcWrite() const { return (encoding_ >> 14) & 0x1; }
    bool GetHWrite() const { return (encoding_ >> 15) & 0x1; }
    bool GetAluInc() const { return (encoding_ >> 16) & 0x1; }
    bool GetAluInva() const { return (encoding_ >> 17) & 0x1; }
    bool GetAluEnb() const { return (encoding_ >> 18) & 0x1; }
    bool GetAluEna() const { return (encoding_ >> 19) & 0x1; }
    uint8_t GetAluFunc() const { return (encoding_ >> 20) & 0x3; }
    bool GetAluSra1() const { return (encoding_ >> 22) & 0x1; }
    bool GetAluSll8() const { return (encoding_ >> 23) & 0x1; }
    bool GetJamz() const { return (encoding_ >> 24) & 0x1; }
    bool GetJamn() const { return (encoding_ >> 25) & 0x1; }
    bool GetJmpc() const { return (encoding_ >> 26) & 0x1; }
    uint16_t GetNextAddr() const { return (encoding_ >> 27) & 0x1FF; }

private:
	uint64_t encoding_ = 0;
};