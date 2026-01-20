#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " [filename].py\n";
        return 1;
    }

    const char *filename = argv[1];
    std::ifstream inputFile(filename);

    if (!inputFile)
    {
        std::cerr << "Error: could not open file '" << filename << "'\n";
        return 1;
    }

    // Read entire file into a string using iterator trick
    std::string source((std::istreambuf_iterator<char>(inputFile)), {});

    std::cout << "Read " << source.length() << " bytes\n";

    return 0;
}