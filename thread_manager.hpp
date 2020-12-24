#include "parser.hpp"
#include "constants.hpp"
#include <thread>
#include <queue>
#include <vector>

class SiteThreadManager {
    private:	
	size_t max_num_threads;
	std::map<std::string, std::map<int, Song>> extracted_data;
	std::vector<std::vector<SiteInfo>> all_sites;
	std::queue<std::vector<SiteInfo>> execution_queue;
    public:
	void load_site_info(SiteInfo info);
	void execute_next_thread();
	void execute_all();
	std::map<std::string, std::map<int, Song>>* get_extracted_data();
};
