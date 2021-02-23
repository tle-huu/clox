#include "chunk.hpp"
#include "vm.hpp"

#include <fstream>
#include <iostream>
#include <streambuf>

static void repl()
{
    VM vm;
    while (true)
    {
        std::cout << "\033[1;32mlox ➜  \033[0m";
        std::string line;
        std::getline(std::cin, line);

        vm.interpret(line);
    }

};

std::string const read_file(std::string const file_path)
{
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);

    if (file)
    {
        std::string buffer;
        file.seekg(0, std::ios::end);   
        buffer.reserve(file.tellg());
        file.seekg(0, std::ios::beg);
        buffer.assign((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

        return buffer;
    }
    else
    {
        std::cerr << "read_file: error opening file " << file_path << '\n';
    }
    return "";
};

int main(int argc, char const * argv[])
{

    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        std::string buffer = read_file(argv[1]);
        VM vm;

        vm.interpret(buffer);
    }
    else
    {
        std::cerr << "Usage: clox [path]\n";
        return 64;
    }

    return 0;
}
