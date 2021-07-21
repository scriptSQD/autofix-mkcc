#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>

// external
#include "argh.h"

std::string tmp = "";

std::string replaceValue(std::string replaceIn, std::string varName, std::string replaceTo) {
    if (varName != "CONFIG_UEVENT_HELPER_PATH") {
        replaceIn.replace(replaceIn.find(varName) + varName.size() + 1, 1, replaceTo);
        return replaceIn;
    }
    else {
        replaceIn.replace(replaceIn.find(varName) + varName.size() + 1, 1, "\"\"");
        return replaceIn;
    }
}

std::string setValue(std::string setIn, std::string varName, std::string setTo) {
    if (setIn.find(varName + " is not set") != std::string::npos) {
        setIn.replace( (setIn.find(varName) + varName.length() ), 11, "="+setTo);
        setIn.replace( (setIn.find(varName) - 2), 2, "");
        return setIn;
    }
    else {
        tmp = varName + "=" + setTo + "\n";
        setIn.append(tmp);
        return setIn;
    }
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");
    std::string targetArg;
    std::filesystem::path targetConfig;
    std::filesystem::path execPath(std::filesystem::path(argv[0]).parent_path());
    system( ( "cd " + execPath.string() ).c_str() );
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
                // std::cout << targetConfig.string() << std::endl;        // debug line
            }
            else {
                fprintf(stderr, "Target does not exist!\n");
            }
        }
        if (arg[{"-v", "--verbose"}]) {
            // std::cout << "Verbose mode enabled\n";        // debug line
            verbose = true;
        }
        if (arg[{"-e", "--errors-only"}]) {
            // std::cout << "Fixing only \"ERROR\" entries...\n";        // debug line
            erronly = true;
        }
    }

    std::string mkvc = ("mer_kcc/mer_verify_kernel_config " + targetConfig.string() + " > " + "mkvc_res.txt");
    system(mkvc.c_str());

    std::ifstream res("mkvc_res.txt", std::ios::in | std::ios::binary);
    if (!res.is_open()) {
        fprintf(stderr, "Failed to open \"mer-kernel-verify-config\" output (mkvc_res.txt)");
        return -1;
    }
    std::string mkvcOut(std::istreambuf_iterator<char>(res), std::istreambuf_iterator<char>{});
    res.close();

    std::regex regex(erronly ? "ERROR:\\s(.*) is invalid\\n(.*)\\nAllowed values\\s*:\\s*(.).*\\n.*" : ":\\s(.*) is invalid\\n(.*)\\nAllowed values\\s*:\\s*(.).*\\n.*");
    std::smatch match;

    std::ifstream kernelCfg(targetConfig.string(), std::ios::in | std::ios::binary);
    std::string cfg (std::istreambuf_iterator<char>(kernelCfg), std::istreambuf_iterator<char>{});
    cfg.append("\n");
    kernelCfg.close();

    while (std::regex_search(mkvcOut, match, regex)) {
        // match[1-3] contain: 1: kernel config var name, 2: cause of error/warning, 3:allowed value nm.1
        if (match[2].str().find("unset") != std::string::npos) {
            cfg = setValue(cfg, match[1].str(), match[3].str());
        }
        else {
            cfg = replaceValue(cfg, match[1].str(), match[3].str());
        }
        if (verbose) {
            std::cout << "Either setting or replacing " << match[1].str() << " value to " << match[3].str() << ".\n";
        }
        mkvcOut = match.suffix().str();
    }
    std::cout << cfg << std::endl;
    if (!std::filesystem::exists("output")) {
        if (!std::filesystem::create_directory(std::filesystem::path("output"))) {
            fprintf(stderr, "Failed to create \"output\" directory...\n");
            return -1;
        }
    }
    std::ofstream cfgOut((execPath / "output" / "defconfig").string(), std::ios::out | std::ios::binary);
    cfgOut << cfg;
    cfgOut.close();

    return 0;
}