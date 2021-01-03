#include <string>
#include <vector>
#include <exception>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "output_writer.hpp"

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
void add_all_sites(std::vector<enum SITE> *sites_to_parse)
{
    sites_to_parse->push_back(GENIE);
    sites_to_parse->push_back(BUGS);
    sites_to_parse->push_back(GAON_STREAMING);
    sites_to_parse->push_back(GAON_DOWNLOAD);
    sites_to_parse->push_back(GAON_DIGITAL);
    sites_to_parse->push_back(MELON);
}

// Adds a single site based off the string. If none match, print and throw exception
void add_site(std::vector<enum SITE> *sites_to_parse, std::string site_string)
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
std::vector<enum SITE> process_sites(int argc, char *argv[]) 
{
    std::vector<enum SITE> sites_to_parse;
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
    
    SiteThreadManager thread_manager;
    // Payload to parse
    std::vector<enum SITE> sites_to_parse;

    // See if all the arguments are correctly spelled
    try {
	sites_to_parse = process_sites(argc, argv);
    } catch (ArgumentNotFoundException &e) {
	std::cout << e.what() << std::endl;
	return EXIT_FAILURE;
    }

    // MELON
    /*
    int number_of_weeks = 0;
    std::stringstream start_buffer, end_buffer;
    
    auto ymd_start {std::chrono::day(12)/std::chrono::April/2010};
    auto sd_start = std::chrono::sys_days{ymd_start};
    
    do {
	MelonInfo info;
	
	start_buffer.str(std::string());
	end_buffer.str(std::string());

	auto sd_end = sd_start + std::chrono::days(6);

	std::time_t starting_date_c = std::chrono::system_clock::to_time_t(sd_start);
	std::tm curr_start_date = *std::localtime(&starting_date_c);
	
	std::time_t ending_date_c = std::chrono::system_clock::to_time_t(sd_end);
	std::tm curr_end_date = *std::localtime(&ending_date_c);

	start_buffer << std::put_time(&curr_start_date, "%Y%m%d");
	end_buffer << std::put_time(&curr_end_date, "%Y%m%d");

	info.start_date = start_buffer.str();
	info.end_date = end_buffer.str();

	thread_manager.load_site_info(&info);

	if (start_buffer.str().compare("20120805") == 0) 
	    sd_start += std::chrono::days(8);
	else
	    sd_start += std::chrono::days(7);

	number_of_weeks++;

    } while (start_buffer.str().compare("20201214") != 0);

    std::cout << "Number of weeks: " << number_of_weeks << std::endl;
    */


    for (int i = 17; i < 53; ++i) {
	std::shared_ptr<GaonInfo> info = std::make_shared<GaonInfo>();
	info->site_type = GAON_DIGITAL;
	info->week = i;
	info->year = 2010;
	info->type = DIGITAL;
	thread_manager.load_site_info(info);
    }

    thread_manager.load_all_sites_to_execution_queue();
    thread_manager.execute_all();
    std::map<std::string, std::map<int, Song*>>* data = thread_manager.get_extracted_data();
    
    for (auto const& [start_date, song_info] : *data) {
	std::cout << "For date: " << start_date << std::endl;
	for (auto const& [rank, song] : song_info) {
	    GaonSong *gaon_song = dynamic_cast<GaonSong*>(song);
	    std::cout << rank << " : " << gaon_song->title << " - " << gaon_song->gaon_index << std::endl;  
	}
    }

    OutputWriter writer(MELON);
    
    return EXIT_SUCCESS;
}
