#include "output_writer.hpp"
#include <filesystem>
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

void output_html(std::shared_ptr<SiteInfo> site_info, std::vector<std::string> htmls)  
{
    SITE type = site_info->site_type;
    Parser *parser;    
    switch(type) {
	case MELON:
	    {
		std::filesystem::create_directories("melon");
		std::string path = "melon/" + site_info->start_date + ".html";
		OutputWriter::write_html(path, htmls[0]);
		break;
	    }
	case GAON_DIGITAL:
	    {
		std::filesystem::create_directories("gaon_digital/" + site_info->start_date);
		std::string path = "gaon_digital/" + site_info->start_date + "/" + site_info->start_date + ".html";
		OutputWriter::write_html(path, htmls[0]);
		break;
	    }
	case BUGS:
	    {
		std::filesystem::create_directories("bugs");
		std::string path = "bugs/" + site_info->start_date + ".html";
		OutputWriter::write_html(path, htmls[0]);
		break;
	    }
	case GENIE:
	    {
		std::string path = "genie/" + site_info->start_date + "/";
		std::filesystem::create_directories(path);
		for (int i = 0; i < htmls.size(); ++i) {
		    std::string page_path = path + std::to_string(i + 1);
		    page_path += ".html";
		    OutputWriter::write_html(page_path, htmls[i]);
		}
		break;
	    }
	case GAON_DOWNLOAD:
	    {
		std::filesystem::create_directories("gaon_download/" + site_info->start_date);
		std::string path = "gaon_download/" + site_info->start_date + "/" + site_info->start_date + ".html";
		OutputWriter::write_html(path, htmls[0]);
		break;
	    }
	case GAON_STREAMING:
	    {
		std::filesystem::create_directories("gaon_streaming/" + site_info->start_date);
		std::string path = "gaon_streaming/" + site_info->start_date + "/" + site_info->start_date + ".html";
		OutputWriter::write_html(path, htmls[0]);
		break;
	    }
	default:
	    {
		std::filesystem::create_directories("gaon_digital/" + site_info->start_date);
		std::string path = "gaon_digital/" + site_info->start_date + "/" + site_info->start_date + ".html";
		OutputWriter::write_html(path, htmls[0]);
		break;
	    }
    }
}

void write_song_outputs(std::shared_ptr<GaonSong> gaon_song, std::string path)
{
    for (auto const& [type, html] : gaon_song->site_htmls) {
	if (type == MELON) {
	    std::string melon_path = path + "melon.html";
	    OutputWriter::write_html(melon_path, html);
	} else if (type == BUGS) {
	    std::string melon_path = path + "bugs.html";
	    OutputWriter::write_html(melon_path, html);
	} else if (type == GENIE) {
	    std::string melon_path = path + "genie.html";
	    OutputWriter::write_html(melon_path, html);
	}
    }
}

void output_song_htmls(std::shared_ptr<SiteInfo> site_info, std::map<int, std::shared_ptr<Song>> data)  
{
    for (auto const& [rank, song] : data) {
	if (std::shared_ptr<GaonSong> gaon_song = std::dynamic_pointer_cast<GaonSong>(song)) {
	    SITE type = site_info->site_type;
	    if (type == GAON_DIGITAL) {
		std::string path = "gaon_digital/" + site_info->start_date + "/" + std::to_string(gaon_song->rank) + "/";
		std::filesystem::create_directories(path);
		write_song_outputs(gaon_song, path);
	    } else if (type == GAON_DOWNLOAD) {
		std::string path = "gaon_download/" + site_info->start_date + "/" + std::to_string(gaon_song->rank) + "/";
		std::filesystem::create_directories(path);
		write_song_outputs(gaon_song, path);
	    } else if (type == GAON_STREAMING) {
		std::string path = "gaon_streaming/" + site_info->start_date + "/" + std::to_string(gaon_song->rank) + "/";
		std::filesystem::create_directories(path);
		write_song_outputs(gaon_song, path);
	    }
	}
    }
}

void SiteThreadManager::execute_next_thread()
{
    std::vector<std::shared_ptr<SiteInfo>> current_thread = execution_queue.front();
    execution_queue.pop();
    active_threads.push_back(std::thread([this](std::vector<std::shared_ptr<SiteInfo>> current_thread) {
		SITE type = current_thread.front()->site_type;
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
		for (auto site_info : current_thread) {
		    std::vector<std::string> htmls;
		    std::string html = parser->request_html(site_info->url);
		    htmls.push_back(html);
		    if (type == GAON_DIGITAL || type == GAON_DOWNLOAD || type == GAON_STREAMING) {
			GaonParser *gaon_parser = dynamic_cast<GaonParser *>(parser);
			if (html.length() > 0) {
			    gaon_parser->extract_dates(site_info, html.c_str());
			    if (site_info->start_date.length() < 1 || site_info->end_date.length() < 1) {
				std::cout << "Date not found" << std::endl;
				continue;
			    }
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
			    
			    htmls.push_back(html_2);
			    htmls.push_back(html_3);
			    htmls.push_back(html_4);

			    std::map<int, std::shared_ptr<Song>> data_pg2 = parser->parse(html_2.c_str());
			    std::map<int, std::shared_ptr<Song>> data_pg3 = parser->parse(html_3.c_str());
			    std::map<int, std::shared_ptr<Song>> data_pg4 = parser->parse(html_4.c_str());
			    
			    data.insert(data_pg2.begin(), data_pg2.end());
			    data.insert(data_pg3.begin(), data_pg3.end());
			    data.insert(data_pg4.begin(), data_pg4.end());
			}
		    }
		    
		    std::cout << "Done: " << site_info->start_date << std::endl;
		    extracted_data.insert(std::pair<std::shared_ptr<SiteInfo>, std::map<int, std::shared_ptr<Song>>>(site_info, data));
		    std::cout << "Extracted data size: " << extracted_data.size() << std::endl;
		    std::cout << "Writing output html..." << std::endl;
		    
		    output_html(site_info, htmls);
		    
		    if (type == GAON_DIGITAL || type == GAON_DOWNLOAD || type == GAON_STREAMING) {
		    	output_song_htmls(site_info, data);
		    }
		}
		delete parser;
		parser = nullptr;
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
