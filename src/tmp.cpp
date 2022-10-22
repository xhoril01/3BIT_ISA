#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <regex>
#include <fstream>

int main(int argc, char** argv)
{
    std::string path (argv[argc-1]);

    std::ifstream file;
    file.open(path);

    std::string data;

    while(getline(file, data))
    {
        std::cout <<"size: " <<data.size() << std::endl;
        for(size_t i = 0; i < data.size(); i++)
        {
            printf("%x\n",data[i]);
        }
    }
}