#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "mic.h"

constexpr uint32_t SEGMENT_CHUNK = 1024; // 1 KB
constexpr uint32_t PROGRAM_SEGMENT_SIZE = SEGMENT_CHUNK;
constexpr uint32_t CONSTANT_SEGMENT_SIZE = SEGMENT_CHUNK;
constexpr uint32_t DATA_SEGMENT_SIZE = SEGMENT_CHUNK;

static void PrintError(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << "\n";
    std::exit(1);
}

class Args {
public:
    Args(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];

            if (a == "-h" || a == "--help") {
                flags_["help"] = true;
            }
            else if (a == "-i" || a == "--input") {
                if (i + 1 >= argc) PrintError("missing value for " + a);
                opts_["input"] = argv[++i];
            }
            else if (a == "-o" || a == "--output") {
                if (i + 1 >= argc) PrintError("missing value for " + a);
                opts_["output"] = argv[++i];
            }
            else if (a == "-p" || a == "--program-size") {
                if (i + 1 >= argc) PrintError("missing value for " + a);
                opts_["program-size"] = argv[++i];
            }
            else if (a == "-c" || a == "--constant-size") {
                if (i + 1 >= argc) PrintError("missing value for " + a);
                opts_["constant-size"] = argv[++i];
            }
            else if (a == "-d" || a == "--data-size") {
                if (i + 1 >= argc) PrintError("missing value for " + a);
                opts_["data-size"] = argv[++i];
            }
            else if (a == "-k" || a == "--constants") {
                if (i + 1 >= argc) PrintError("missing value for " + a);
                opts_["constants"] = argv[++i];
            }
            else if (a == "--max-cycles") {
                if (i + 1 >= argc) PrintError("missing value for " + a);
                opts_["max-cycles"] = argv[++i];
            }
            else if (!a.empty() && a[0] == '-') {
                PrintError("unknown option: " + a);
            }
            else {
                PrintError("unexpected argument: " + a);
            }
        }
    }

    bool CheckFlag(const std::string& name) const {
        auto it = flags_.find(name);
        return it != flags_.end() && it->second;
    }

    std::string GetOption(
            const std::string& name, const std::string& def = "") const {
        auto it = opts_.find(name);
        return it == opts_.end() ? def : it->second;
    }

private:
    std::map<std::string, bool> flags_;
    std::map<std::string, std::string> opts_;
};

void PrintHelp() {
    std::cout << "micsim -i program.bin -o output.bin [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t--input (-i)         Input program binary file name" << std::endl;
    std::cout << "\t--help (-h)          Print help message" << std::endl;
    std::cout << "\t--output (-o)        Result data segment file name" << std::endl;
    std::cout << "\t--program-size (-p)  Program segment size (bytes)" << std::endl;
    std::cout << "\t--constant-size (-c) Constants segment size (bytes)" << std::endl;
    std::cout << "\t--data-size (-d)     Data segment size (bytes)" << std::endl;
    std::cout << "\t--constants (-k)     Constants segment file name" << std::endl;
    std::cout << "\t--max-cycles         Abort after N cycles (0 = no limit)" << std::endl;
}

std::vector<uint8_t> ReadBinary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        PrintError("can't open " + filename);
    }

    size_t size = file.tellg();
    if (size == 0) {
        PrintError("file " + filename + " is empty");
    }

    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (size > 0 && !file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        PrintError("read failed for " + filename);
    }

    return buffer;
}

void WriteBinary(
        const std::string& filename,
        const std::vector<uint32_t>& data) {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file) {
        PrintError("can't open " + filename + " for writing");
    }

    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
    }

    file.close();
    if (!file) {
        PrintError("write failed: " + filename);
    }
}

int main(int argc, char* argv[]) {
    Args args(argc, argv);
    if (args.CheckFlag("help")) {
        PrintHelp();
        return 0;
    }

    auto program_file = args.GetOption("input");
    if (program_file.empty()) {
        PrintHelp();
        PrintError("no program file");
    }
    auto program = ReadBinary(program_file);

    auto output_file = args.GetOption("output");
    if (output_file.empty()) {
        PrintHelp();
        PrintError("no output file");
    }

    uint32_t program_segment_size = PROGRAM_SEGMENT_SIZE;
    auto program_size = args.GetOption("program-size");
    if (!program_size.empty()) {
        program_segment_size = std::stoul(program_size);
    }
    if (program.size() > program_segment_size) {
        program_segment_size =
            ((program.size() + SEGMENT_CHUNK - 1) / SEGMENT_CHUNK)
            * SEGMENT_CHUNK;
    }

    uint32_t constant_segment_size = CONSTANT_SEGMENT_SIZE;
    auto constant_size = args.GetOption("constant-size");
    if (!constant_size.empty()) {
        constant_segment_size = std::stoul(constant_size);
    }
    uint32_t data_segment_size = DATA_SEGMENT_SIZE;
    auto data_size = args.GetOption("data-size");
    if (!data_size.empty()) {
        data_segment_size = std::stoul(data_size);
    }

    GlobalMemory memory(
        program_segment_size, constant_segment_size, data_segment_size);
    memory.SetProgram(program);

    auto constants_file = args.GetOption("constants");
    if (!constants_file.empty()) {
        std::ifstream file(constants_file, std::ios::binary | std::ios::ate);
        if (!file) {
            PrintError("can't open " + constants_file);
        }
        size_t size = file.tellg();
        if (size > 0) {
            if (size % sizeof(uint32_t) != 0) {
                PrintError(
                    "constants file size is not a multiple of 4 bytes: "
                    + constants_file);
            }
            if (size > constant_segment_size) {
                PrintError(
                    "constants file is larger than the constants segment: "
                    + constants_file);
            }
            file.seekg(0, std::ios::beg);
            std::vector<uint8_t> raw(size);
            if (!file.read(reinterpret_cast<char*>(raw.data()), size)) {
                PrintError("read failed for " + constants_file);
            }
            std::vector<uint32_t> constants(size / sizeof(uint32_t));
            memcpy(constants.data(), raw.data(), size);
            memory.SetConstants(constants);
        }
    }

    uint64_t max_cycles = 0;
    auto max_cycles_opt = args.GetOption("max-cycles");
    if (!max_cycles_opt.empty()) {
        max_cycles = std::stoull(max_cycles_opt);
    }

    MIC mic(&memory);

    uint64_t cycle_count = 0;
    while (true) {
        ++cycle_count;
        if (mic.Cycle()) break;
        if (max_cycles != 0 && cycle_count >= max_cycles) {
            PrintError(
                "max cycles reached (" + std::to_string(max_cycles) + ")");
        }
    }

    auto data_segment_elementes = data_segment_size / sizeof(uint32_t);
    auto output = memory.GetData(data_segment_elementes);
    WriteBinary(output_file, output);

    std::cout << "Cycles completed: " << cycle_count
              << " (stall: " << mic.GetStallCycles() << ")" << std::endl;
    // 1 simulator cycle = 3 micro-clocks: the time in micro-clocks.
    std::cout << "Micro-clocks completed: " << cycle_count * 3
              << " (stall: " << mic.GetStallCycles() * 3 << ")" << std::endl;
    std::cout << "Output written: " << output_file << std::endl;
}