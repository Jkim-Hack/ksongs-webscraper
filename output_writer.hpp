#include <string>
#include <map>
#include "melon_parser.hpp"
#include "constants.hpp"

class OutputWriter {
    private:
	Site site_to_use;
	std::string filepath;
    public:
	OutputWriter(): site_to_use(MELON), filepath("output.csv"){};
	OutputWriter(Site site_to_use): site_to_use(site_to_use), filepath("output.csv"){};
	OutputWriter(Site site_to_use, std::string filepath): site_to_use(site_to_use), filepath(filepath){};
	void set_output(std::string filepath);
	void set_site_to_use(Site site_to_use);
	void execute_output(std::map<std::string, std::map<int, Song>> data);
};
