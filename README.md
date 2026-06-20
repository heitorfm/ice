# Ice Configuration Parser

A C++ parser for the Ice configuration language - a domain-specific language (DSL) for defining structured configuration files with support for sections, assignments, arrays, objects, functions, and control flow statements.

## Features

- **Sections**: Organize configuration into hierarchical sections
- **Data Types**: Support for strings, numbers (int/double), booleans, null, arrays, and objects
- **Functions**: Function calls with parameters
- **Control Flow**: if/for statements for conditional and iterative logic
- **Math Expressions**: Basic arithmetic operations (currently commented out, see improvements)
- **Debug Output**: Built-in debugging and pretty-printing capabilities

## Requirements

- C++ compiler (g++ recommended)
- [re2c](https://re2c.org/) - lexer generator
- [Lemon](https://www.hwaci.com/sw/lemon/) - parser generator
- Make

## Building

1. Ensure re2c and lemon are installed on your system
2. From the project root, run make to build the project:
   ```bash
   make build
   ```
   This will:
   - Generate lexer and parser files in the `build/` directory
   - Compile source files and place object files in `build/`
   - Create the static library `dist/libice.a`

## Usage

### As a Library

Include the headers and link against `dist/libice.a`:

```cpp
#include "src/Ice.hpp"

// Parse a configuration file
FILE* fp = fopen("config.ice", "r");
IceConfig* config = Ice::Ice::parseFile(fp, true); // true for debug output
fclose(fp);

// Access parsed configuration
// config->sections contains the parsed sections
```

When compiling your application:
```bash
g++ -std=c++11 -I. your_app.cpp dist/libice.a -o your_app
```

### Command Line Tool

The project includes a main executable that parses a `test.ice` file (you'll need to create this):

```bash
# Create a test configuration file
echo 'ICE-1.0.0
realm >= 1.0.0

@database {
    host: "localhost"
    port: 5432
    enabled: true
}

@features {
    list: ["auth", "logging", "cache"]
    config: {
        timeout: 30
        retries: 3
    }
}' > simple.ice

# Build and run
make main
./dist/ice  # This will parse simple.ice
```

## Configuration Syntax

### Basic Structure
```
ICE-<version>
realm >= <version>

@section_name {
    key: value
}
```

### Data Types
```ice
# Strings
name: "John Doe"

# Numbers
port: 8080
timeout: 30.5

# Booleans
enabled: true
debug: false

# Null
optional: null

# Arrays
tags: ["web", "api", "microservice"]

# Objects
config: {
    host: "localhost"
    port: 5432
}
```

### Functions
```ice
result = mynamespace->calculate(x, y)
```

### Control Flow
```ice
if enabled {
    debug = true
}

for item in items {
    process(item)
}
```

## Project Structure

```
src/
├── Ice.hpp/cpp          # Main API
├── Nodes.hpp/cpp        # AST node definitions
├── DebugParser.hpp/cpp  # Debug output utilities
├── StrUtil.hpp/cpp      # String utilities
├── IntUtil.hpp/cpp      # Integer parsing utilities
├── IceApi.re            # Lexer definition (re2c)
├── icelang.y            # Parser grammar (Lemon)
└── main.cpp             # Example usage
gen/                     # Generated source files
├── IceApi.cpp          # Generated lexer
├── icelang.c/.h        # Generated parser
└── icelang.out         # Parser debug info
build/                   # Object files
└── *.o                 # Compiled object files
dist/                    # Distribution files
├── libice.a            # Static library
└── ice_parser          # Executable
Makefile                # Build configuration
README.md               # This file
improvements.md         # Known issues and improvements
```

## Development

### Code Style
- Use PascalCase for classes and files
- Use camelCase for functions and variables
- Write comments in English

### Known Issues & Improvements

See [improvements.md](improvements.md) for a detailed list of areas that need enhancement, including:
- Memory management improvements
- Modern C++ adoption
- Better error handling
- Cross-platform build support
- Unit tests and documentation

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Ensure the code compiles and works
5. Submit a pull request

## License

This project is provided as-is without any specific license. Please check with the original author for usage permissions.