#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "constants.h"

// If the site cannot be found, throw this exception
class ArgumentNotFoundException : public std::exception {
    std::string argument;

    public:
    	ArgumentNotFoundException(std::string argument) {
	    this->argument = argument;
	}
	const char* what() const throw() {
	    std::string message("Site could not be found: ");
	    message += argument;
	    return message.c_str();
	}
};

// Checks if there's the right number of args
int arg_checker(int argc)
{
    if (argc < 3) {
      return 0;
    }   
    return 1;
}

// Adds all the sites into the parser payload
void add_all_sites(std::vector<enum Site> *sites_to_parse)
{
    sites_to_parse->push_back(GENIE);
    sites_to_parse->push_back(BUGS);
    sites_to_parse->push_back(GAON_STREAMING);
    sites_to_parse->push_back(GAON_DOWNLOAD);
    sites_to_parse->push_back(GAON_DIGITAL);
    sites_to_parse->push_back(MELON);
}

// Adds a single site based off the string. If none match, print and throw exception
void add_site(std::vector<enum Site> *sites_to_parse, std::string site_string)
{
    if (!site_string.compare("melon")) {
	sites_to_parse->push_back(MELON);
    } else if (!site_string.compare("gaon")) {
	sites_to_parse->push_back(GAON_DIGITAL);
	sites_to_parse->push_back(GAON_DOWNLOAD);
	sites_to_parse->push_back(GAON_STREAMING);
    } else if (!site_string.compare("gaon_digital")) {
	sites_to_parse->push_back(GAON_DIGITAL);
    } else if (!site_string.compare("gaon_download")) {
	sites_to_parse->push_back(GAON_DOWNLOAD);
    } else if (!site_string.compare("gaon_streaming")) {
	sites_to_parse->push_back(GAON_STREAMING);
    } else if (!site_string.compare("bugs")) {
	sites_to_parse->push_back(BUGS);
    } else if (!site_string.compare("genie")) {
	sites_to_parse->push_back(GENIE);
    } else {
	throw ArgumentNotFoundException(site_string);
    }
}

// Processes all sites and places them into a payload to parse
std::vector<enum Site> process_sites(int argc, char *argv[]) 
{
    std::vector<enum Site> sites_to_parse;
    std::string first(argv[1]);
    if (first.compare("all") == 0) {
	add_all_sites(&sites_to_parse);
    } else {
	for (int i = 1; i < argc - 1; ++i) {
	    std::string site(argv[i]);
	    add_site(&sites_to_parse, site);
	}
    }
    return sites_to_parse;
}

int main(int argc, char *argv[])
{
    // Check to make sure the input has the correct number of arguments
    if (!arg_checker(argc)) {
      std::cout << "usage: ksongs-webscraper <sites> <file ext>" << std::endl;
      return EXIT_SUCCESS;
    }
    
    // Payload to parse
    std::vector<enum Site> sites_to_parse;

    // See if all the arguments are correctly spelled
    try {
	sites_to_parse = process_sites(argc, argv);
    } catch (ArgumentNotFoundException &e) {
	std::cout << e.what() << std::endl;
	return EXIT_FAILURE;
    }

    // TEST: print all the parsed sites
    for (const auto &site : sites_to_parse) {
	std::cout << site << std::endl;
    }

    return EXIT_SUCCESS;
}
