#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "mic.h"

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
    std::cout << "\t--input (-i) \tInput program binary" << std::endl;
    std::cout << "\t--help (-h)  \tPrint help message" << std::endl;
    std::cout << "\t--output (-o)\tGlobal memory snapshot" << std::endl;
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

int main(int argc, char* argv[]) {
    Args args(argc, argv);
    if (args.CheckFlag("help")) {
        PrintHelp();
    }

    auto program = args.GetOption("input");
    if (program.empty()) {
        PrintHelp();
        PrintError("no program file");
    }

    auto output = args.GetOption("output");
    if (output.empty()) {
        PrintHelp();
        PrintError("no output file");
    }


}