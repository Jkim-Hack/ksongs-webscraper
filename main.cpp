#include <iostream>
#include <string>
#include <curl/curl.h>

int arg_checker(int argc)
{
    if (argc < 3) {
      return 0;
    }   
    return 1;
}

int main(int argc, char *argv[])
{
    if (!arg_checker(argc)) {
      std::cout << "usage: ksongs-webscraper <sites> <file ext>" << std::endl;
      return 0;
    }

    return 0;
}
