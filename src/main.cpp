#include <iostream>
#include <cstdio>
#include <string>
#include <chrono>
#include <vector>
#include "IceParser.hpp"

// Structure to hold parsed command line arguments
struct CliArgs {
    std::string inputFile;
    bool checkOnly;
    bool showConfig;
    bool info;
    bool debugParser;
    bool showHelp;
    MissingVariableBehavior missingVariableBehavior;
};

// Forward declaration for printUsage
void printUsage(const char* programName);

const char* errorCodeName(IceErrorCode code) {
    switch (code) {
        case IceErrorCode::SyntaxError: return "SyntaxError";
        case IceErrorCode::ConditionalError: return "ConditionalError";
        case IceErrorCode::ReferenceError: return "ReferenceError";
        case IceErrorCode::InterpolationError: return "InterpolationError";
        case IceErrorCode::MathError: return "MathError";
        case IceErrorCode::ReadError: return "ReadError";
        case IceErrorCode::InternalError: return "InternalError";
    }

    return "UnknownError";
}

void printErrors(const IceParserResult& result) {
    for (const IceError& error: result.getErrors()) {
        std::cerr << errorCodeName(error.code()) << ": " << error.message();
        if (error.line() > 0) {
            std::cerr << " on line " << error.line();
        }
        std::cerr << std::endl;
    }
}

MissingVariableBehavior parseMissingVariableBehavior(const std::string& value, const char* programName) {
    if (value == "error") {
        return MissingVariableBehavior::Error;
    }

    if (value == "blank") {
        return MissingVariableBehavior::Blank;
    }

    if (value == "var-name") {
        return MissingVariableBehavior::VarName;
    }

    std::cerr << "Error: Invalid --missing-var value '" << value << "'" << std::endl;
    printUsage(programName);
    exit(1);
}

// Function to parse command line arguments
CliArgs parseArguments(int argc, char* argv[]) {
    CliArgs args = {"", false, false, true, false, false, MissingVariableBehavior::Error};

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-f" || arg == "--file") {
            if (i + 1 < argc) {
                args.inputFile = argv[++i];
            } else {
                std::cerr << "Error: -f/--file requires a filename argument" << std::endl;
                exit(1);
            }
        } else if (arg == "-c" || arg == "--check") {
            args.checkOnly = true;
            args.showConfig = false;
        } else if (arg == "--show") {
            args.showConfig = true;
        } else if (arg == "--no-show") {
            args.showConfig = false;
        } else if (arg == "--info") {
            args.info = true;
        } else if (arg == "--no-info") {
            args.info = false;
        } else if (arg == "--debug") {
            args.debugParser = true;
        } else if (arg == "--missing-var") {
            if (i + 1 < argc) {
                args.missingVariableBehavior = parseMissingVariableBehavior(argv[++i], argv[0]);
            } else {
                std::cerr << "Error: --missing-var requires a value: error, blank, or var-name" << std::endl;
                exit(1);
            }
        } else if (arg == "-h" || arg == "--help") {
            args.showHelp = true;
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
            printUsage(argv[0]);
            exit(1);
        }
    }

    return args;
}

// Function to validate arguments
bool validateArguments(const CliArgs& args) {
    if (args.inputFile.empty()) {
        std::cerr << "Error: Input file is required. Use -f or --file to specify a file." << std::endl;
        std::cerr << "Use -h or --help for usage information." << std::endl;
        return false;
    }
    return true;
}

void printUsage(const char* programName) {
    std::cout << "Ice Configuration Parser CLI" << std::endl;
    std::cout << "Usage: " << programName << " -f FILE [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Required:" << std::endl;
    std::cout << "  -f, --file FILE    Input file to parse" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --check        Check syntax only, don't show debug output" << std::endl;
    std::cout << "  --show             Show parsed config output" << std::endl;
    std::cout << "  --no-show          Hide parsed config output" << std::endl;
    std::cout << "  --info             Show CLI info output" << std::endl;
    std::cout << "  --no-info          Hide CLI info output" << std::endl;
    std::cout << "  --debug            Enable parser debug output" << std::endl;
    std::cout << "  --missing-var MODE Handle missing interpolations: error, blank, var-name" << std::endl;
    std::cout << "  -h, --help         Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " -f config.ice       # Parse config.ice" << std::endl;
    std::cout << "  " << programName << " -f config.ice -c    # Check config.ice syntax" << std::endl;
    std::cout << "  " << programName << " --file app.ice      # Parse app.ice" << std::endl;
}

int main(int argc, char* argv[]) {

    using namespace Ice;

    // Parse command line arguments
    CliArgs args = parseArguments(argc, argv);

    if (args.showHelp) {
        printUsage(argv[0]);
        return 0;
    }

    // Validate arguments
    if (!validateArguments(args)) {
        return 1;
    }

    // Open input file
    FILE* fp = fopen(args.inputFile.c_str(), "r");
    if (fp == NULL) {
        std::cerr << "Error: Can't open file '" << args.inputFile << "'" << std::endl;
        return 1;
    }

    if (args.info) {
        std::cout << "Parsing file: " << args.inputFile << std::endl;
    }

    // Start timing
    auto startTime = std::chrono::high_resolution_clock::now();

    IceParserParams parserParams;
    parserParams.printConfig = args.showConfig;
    parserParams.debugParser = args.debugParser;
    parserParams.info = args.info;
    parserParams.missingVariableBehavior = args.missingVariableBehavior;

    // Parse using Ice API
    std::unique_ptr<IceParserResult> result = IceParser::parseFile(fp, parserParams);

    // End timing
    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    fclose(fp);

    if (result && result->isSuccess()) {
        if (args.info) {
            if (args.checkOnly) {
                std::cout << "Syntax check passed" << std::endl;
            } else {
                std::cout << "Parsing completed successfully" << std::endl;
            }
            std::cout << "Parsing time: " << duration.count() << "μs" << std::endl;
        }
        return 0;
    } else {
        if (result) {
            printErrors(*result);
        }
        if (args.info) {
            std::cout << "Parsing failed" << std::endl;
            std::cout << "Parsing time: " << duration.count() << "μs" << std::endl;
        }
        return 1;
    }
}
