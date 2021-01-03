#include "thread_manager.hpp"
#include <iostream>

SiteThreadManager::SiteThreadManager() 
{
    max_num_items_per_thread = 100;
    max_thread_dist_size = 53;
}

void SiteThreadManager::load_site_info(std::shared_ptr<SiteInfo> info) {
    long key_hash;
    switch(info->site_type) {
	case MELON:
	    {
		melon_parser.load_info(info, max_thread_dist_size);
		break;
	    }
	case GAON_DIGITAL:
	    {
		gaon_parser.load_info(info, max_thread_dist_size);
		break;
	    }
	case BUGS:
	    {
		// TODO: Implement Bugs
		break;
	    }
	case GENIE:
	    {
		// TODO: Implement Genie
		break;
	    }
	default:
	    break;
    }

    long site_hash = info->key_hash;
    if (all_sites.find(site_hash) != all_sites.end()) {
	// Exists
	all_sites[site_hash].push_back(info);
    } else {
	// Deosn't exist
    	std::vector<std::shared_ptr<SiteInfo>> new_pool;
	new_pool.push_back(info);
	all_sites.insert(std::pair<long, std::vector<std::shared_ptr<SiteInfo>>>(site_hash, new_pool));
    }
}

void SiteThreadManager::load_all_sites_to_execution_queue()
{
    for (auto const& [key, val] : all_sites) {
	std::cout << val.size() << std::endl;
	execution_queue.push(val);
    }
}

void SiteThreadManager::execute_next_thread()
{
    std::vector<std::shared_ptr<SiteInfo>> current_thread = execution_queue.front();
    execution_queue.pop();
    active_threads.push_back(std::thread([this](std::vector<std::shared_ptr<SiteInfo>> current_thread) {
		for (auto site_info : current_thread) {
		    GaonParser gaon_parser;
		    std::string html = gaon_parser.request_html(site_info->url);
		    gaon_parser.extract_dates(site_info, html.c_str());
		    std::cout << "Doing: " << site_info->start_date << std::endl;
		    std::map<int, Song*> data = gaon_parser.parse(html.c_str());
		    std::cout << "Done: " << site_info->start_date << std::endl;
		    extracted_data.insert(std::pair<std::string, std::map<int, Song*>>(site_info->start_date, data));
		    std::cout << "Extracted data size: " << extracted_data.size() << std::endl;
		    site_info.reset();
		}
	}, current_thread));
}

void SiteThreadManager::execute_all()
{
    while (execution_queue.size() != 0) {
	execute_next_thread();
    }
}

std::map<std::string, std::map<int, Song*>>* SiteThreadManager::get_extracted_data()
{
    for (auto&& curr_thread : active_threads) {
	curr_thread.join();
    }
    return &extracted_data;
}
