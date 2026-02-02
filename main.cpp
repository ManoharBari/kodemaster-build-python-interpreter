#include <iostream>
#include <fstream>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

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

    try
    {
        // Lexing
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.scanTokens();

        // Parsing
        Parser parser(tokens);
        ProgramNode *program = parser.parse();

        // Interpreting
        Interpreter interpreter;
        interpreter.interpret(program);

        // Cleanup
        delete program;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}