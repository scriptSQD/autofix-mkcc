#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>

// external
#include "argh.h"

void replaceValue(std::string replaceIn, std::string varName, std::string replaceTo) {
    replaceIn.replace(replaceIn.find(varName) + varName.size() + 1, 1, replaceTo);
}

void setValue(std::string setIn, std::string varName, std::string setTo) {
    setIn.replace(setIn.find(varName) + varName.size(), 11, "="+setTo);
    setIn.replace(setIn.find(varName) - 2, 2, "");
}

int main(int argc, char** argv) {
    std::string targetArg;
    std::filesystem::path targetConfig;
    std::string execPath(std::filesystem::path(argv[0]).parent_path().string());
    bool verbose = false;
    bool erronly = false;
    {   // starter argument check 'n' parse
        argh::parser arg;
        arg.add_params({"-t", "--target"});
        arg.parse(argv);
        if (!(arg({"t", "target"}) >> targetArg)) {
            fprintf(stderr, "No target config provided...\n");
            return -1;
        }
        else {
            targetConfig = targetArg;
            if (std::filesystem::exists(targetConfig)) {
                std::cout << targetConfig.string() << std::endl;        // debug line
            }
            else {
                fprintf(stderr, "Target does not exist!\n");
            }
        }
        if (arg[{"-v", "--verbose"}]) {
            std::cout << "Verbose mode enabled\n";        // debug line
            //somehow define that everything outputs to user
            verbose = true;
        }
        if (arg[{"-e", "--errors-only"}]) {
            std::cout << "Fixing only \"ERROR\" entries...\n";        // debug line
            erronly = true;
        }
    }
    // targetConfig = "/home/sqd/Desktop/halium-repo/kernel/samsung/universal7580/arch/arm64/configs/lineageos_a3xelte_defconfig"; // debug line

    std::string mkvc = (execPath + "mer_kcc/mer_verify_kernel_config " + targetConfig.string() + " > " + execPath + "mkvc_res.txt");
    system(mkvc.c_str());

    std::ifstream res(execPath + "mkvc_res.txt", std::ios::in | std::ios::binary);
    if (!res.is_open()) {
        fprintf(stderr, "Failed to open \"mer-kernel-verify-config\" output");
        return -1;
    }
    std::string mkvcOut(std::istreambuf_iterator<char>(res), std::istreambuf_iterator<char>{});
    res.close();

    std::regex regex(erronly ? R"(ERROR:\s(.*) is invalid\n(.*)\nAllowed values\s*:\s*(.).*\n.*)" : R"(:\s(.*) is invalid\n(.*)\nAllowed values\s*:\s*(.).*\n.*)");
    std::smatch match;

    std::ifstream kernelCfg(targetConfig.string(), std::ios::in | std::ios::binary);
    std::string cfg (std::istreambuf_iterator<char>(kernelCfg), std::istreambuf_iterator<char>{});
    kernelCfg.close();

    while (std::regex_search(mkvcOut, match, regex)) {
        // match[1-3] contain: 1: kernel config var name, 2: cause of error/warning, 3:allowed value nm.1
        mkvcOut = match.suffix().str();
        match[2].str().find("unset") ? setValue(cfg, match[1], match[3]) : replaceValue(cfg, match[1], match[3]);
        if (verbose) {
            std::cout << "Either setting or replacing " + match[1].str() + " value to " + match[3].str() + ".\n";
        }
    }

    if (!std::filesystem::create_directory(std::filesystem::path(argv[0]).parent_path() / "output")) {
        fprintf(stderr, "Failed to create \"output\" directory...\n");
    }
    std::ofstream cfgOut(execPath + "output/" + "defconfig", std::ios::out | std::ios::binary);
    cfgOut << cfg;
    cfgOut.close();

    return 0;
}