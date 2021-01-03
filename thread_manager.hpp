#ifndef __THREAD_MANAGER_HPP__
#define __THREAD_MANAGER_HPP__

#include "parser.hpp"
#include <thread>
#include <queue>
#include <vector>
#include <thread>

class SiteThreadManager {
    private:	
	MelonParser melon_parser;
	GaonParser gaon_parser;
	size_t max_num_items_per_thread;
	size_t max_thread_dist_size;
	size_t num_running_threads;
	size_t num_waiting_threads;
	std::map<std::string, std::map<int, Song*>> extracted_data;
	std::map<long, std::vector<std::shared_ptr<SiteInfo>>> all_sites;
	std::queue<std::vector<std::shared_ptr<SiteInfo>>> execution_queue;
	std::vector<std::thread> active_threads;
    public:
	SiteThreadManager();
	void load_site_info(std::shared_ptr<SiteInfo>);
	void load_all_sites_to_execution_queue();
	void execute_next_thread();
	void execute_all();
	std::map<std::string, std::map<int, Song*>>* get_extracted_data();
};

#endif
