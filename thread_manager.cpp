#include "thread_manager.hpp"
#include <iostream>

SiteThreadManager::SiteThreadManager() 
{
    max_num_items_per_thread = 100;
    max_thread_dist_size = 131;
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
	case GAON_DOWNLOAD:
	    {
		gaon_parser.load_info(info, max_thread_dist_size);
		break;
	    }
	case GAON_STREAMING:
	    {
		gaon_parser.load_info(info, max_thread_dist_size);
		break;
	    }
	case BUGS:
	    {
		bugs_parser.load_info(info, max_thread_dist_size);
		break;
	    }
	case GENIE:
	    {
		genie_parser.load_info(info, max_thread_dist_size);
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
		    SITE type = site_info->site_type;
		    Parser *parser;    
		    switch(type) {
			case MELON:
			    {
				parser = new MelonParser;
				break;
			    }
			case GAON_DIGITAL:
			    {
				parser = new GaonParser;
				break;
			    }
			case BUGS:
			    {
				parser = new BugsParser;
			    	break;
			    }
			case GENIE:
			    {
				parser = new GenieParser;
				break;
			    }
			case GAON_DOWNLOAD:
			    {
				parser = new GaonParser;
				break;
			    }
			case GAON_STREAMING:
			    {
				parser = new GaonParser;
				break;
			    }
			default:
			    {
				parser = new GaonParser;
				break;
			    }
		    }
		    std::string html = parser->request_html(site_info->url);
		    if (type == GAON_DIGITAL || type == GAON_DOWNLOAD || type == GAON_STREAMING) {
			GaonParser *gaon_parser = dynamic_cast<GaonParser *>(parser);
			if (html != "") {
			    gaon_parser->extract_dates(site_info, html.c_str());
			} else {
			    std::cout << "html: " << html;
			}
		    }
		    
		    std::cout << "Doing: " << site_info->start_date << std::endl;
		    std::map<int, std::shared_ptr<Song>> data = parser->parse(html.c_str());
		    
		    if (type == GENIE) {
			if (std::shared_ptr<GenieInfo> info = std::dynamic_pointer_cast<GenieInfo>(site_info)) {
			   
			    std::string html_2 = parser->request_html(info->url_2);
			    std::string html_3 = parser->request_html(info->url_3);
			    std::string html_4 = parser->request_html(info->url_4);
			    
			    std::map<int, std::shared_ptr<Song>> data_pg2 = parser->parse(html_2.c_str());
			    std::map<int, std::shared_ptr<Song>> data_pg3 = parser->parse(html_3.c_str());
			    std::map<int, std::shared_ptr<Song>> data_pg4 = parser->parse(html_4.c_str());
			    
			    data.insert(data_pg2.begin(), data_pg2.end());
			    data.insert(data_pg3.begin(), data_pg3.end());
			    data.insert(data_pg4.begin(), data_pg4.end());
			}
		    }
		    
		    std::cout << "Done: " << site_info->start_date << std::endl;
		    delete parser;
		    parser = nullptr;
		    extracted_data.insert(std::pair<std::shared_ptr<SiteInfo>, std::map<int, std::shared_ptr<Song>>>(site_info, data));
		    std::cout << "Extracted data size: " << extracted_data.size() << std::endl;
		    
		}
	}, current_thread));
}

void SiteThreadManager::execute_all()
{
    while (execution_queue.size() != 0) {
	execute_next_thread();
    }
}

std::map<std::shared_ptr<SiteInfo>, std::map<int, std::shared_ptr<Song>>>* SiteThreadManager::get_extracted_data()
{
    for (auto&& curr_thread : active_threads) {
	curr_thread.join();
    }
    return &extracted_data;
}
