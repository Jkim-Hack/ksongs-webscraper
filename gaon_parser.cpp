#include "gaon_parser.hpp"
#include <iostream>
#include <curl/curl.h>
#include <myhtml/api.h>
#include <algorithm>

std::string GaonParser::generate_url(GaonInfo info) 
{
    std::string url;
    if (info.type == DIGITAL) {
	url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=ALL&targetTime=" + std::to_string(info.week) + "&hitYear=" + std::to_string(info.year) + "&termGbn=week";
    } else if (info.type == DOWNLOAD) {
	url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=S1020&targetTime=" + std::to_string(info.week) + "&hitYear=" + std::to_string(info.year) + "&termGbn=week";
    } else if (info.type == STREAMING) {
	url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=S1040&targetTime=" + std::to_string(info.week) + "&hitYear=" + std::to_string(info.year) + "&termGbn=week";
    } else {
	// TODO: Exception
    }
    return url;
}

void GaonParser::load_info(GaonInfo info)
{
    info.url = generate_url(info);
    site_infos.push_back(info);
}

size_t GaonParser::write(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

void extract_dates(SiteInfo *info, const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t* doc_node = myhtml_tree_get_document(tree);
    myhtml_tree_node_t* root =  myhtml_node_child(doc_node);
    
    myhtml_tree_node_t* selection_node = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_SELECT, NULL)->list[0];

    std::string attr_key = "selected";
    myhtml_tree_node_t *selected_node = myhtml_get_nodes_by_attribute_key(tree, NULL, selection_node, attr_key.c_str(), attr_key.length(), NULL)->list[0];

    myhtml_tree_node_t *text_node = myhtml_node_child(selected_node);
    std::string dates = myhtml_node_text(text_node, NULL);

    size_t delim_pos = dates.find("~");
    std::string start = dates.substr(0, delim_pos);
    std::string end = dates.substr(delim_pos, dates.length() - delim_pos - 1);

    start.erase(std::remove(start.begin(), start.end(), '.'), start.end());
    end.erase(std::remove(end.begin(), end.end(), '.'), end.end());

    info->start_date = start;
    info->end_date = end;

    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
}

// Prepare libcurl for get request
void GaonParser::prepare_handle(int index) 
{
    std::string response_string;
    SiteInfo info = this->site_infos.at(index);
    if (curl) {
	curl_easy_setopt(curl, CURLOPT_URL, info.url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &GaonParser::write);    
	std::cout << info.url << std::endl;
	curl_easy_perform(curl);
	extract_dates(&info, response_string.c_str());
	std::map<int, Song> week_data = GaonParser::parse(response_string.c_str());
	extracted_data.insert(std::pair<std::string, std::map<int, Song>>(info.start_date, week_data));
	std::cout << "Week " << index << " done\n";
    }
}

std::map<std::string, std::string> get_node_attrs(myhtml_tree_node_t *node)
{
    std::map<std::string, std::string> attributes;
    myhtml_tree_attr_t *attr = myhtml_node_attribute_first(node);
    
    while (attr) {
        const char *name = myhtml_attribute_key(attr, NULL);
	if(name) {
            const char *value = myhtml_attribute_value(attr, NULL);
	    attributes.insert(std::pair<std::string, std::string>(name, value));
        }
        
        attr = myhtml_attribute_next(attr);
    }
    return attributes;
}

std::string extract_id(std::string text) 
{
    size_t first = text.find_first_of("'");
    size_t last = text.find_last_of("'");
    return text.substr(first + 1, last-first-1);
}

Song scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) 
{
    Song song_info;
    myhtml_collection_t* td_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tr_node, MyHTML_TAG_TD, NULL);
    for (int i = 0; i < td_nodes->length; ++i) {
	switch(i) {
	    case 0: 
		    {    
			//Get title and id info
			myhtml_tree_node_t *span_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_SPAN, NULL)->list[0];
			myhtml_tree_node_t *inner_text = myhtml_node_child(span_node);
			song_info.rank = std::stoi(myhtml_node_text(inner_text, NULL));
			break;
		    }
	    case 3: 
		    {
			myhtml_collection_t *p_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_SPAN, NULL);
			std::map<std::string, std::string> title_attrs = get_node_attrs(p_nodes->list[0]);
			std::map<std::string, std::string> artist_attrs = get_node_attrs(p_nodes->list[0]);	
			std::string art_alb = artist_attrs["title"];

			size_t delim_pos = art_alb.find("|");
			song_info.artist = art_alb.substr(0, delim_pos);
			song_info.album = art_alb.substr(delim_pos, art_alb.length() - delim_pos - 1);
			song_info.title = title_attrs["title"];
			break;
		    }
		    // TODO: Add more scraping after asking Dr. Kim and Dr. Park
	    case 6:
		    {
			myhtml_collection_t *p_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_P, NULL);
			myhtml_tree_node_t *text_node = myhtml_node_child(p_nodes->list[0]);
		    	break;
		    }
	}
    }
    return song_info;
}

std::map<int, Song> extract_data(myhtml_tree_t* tree, myhtml_tree_node_t *node)
{
    std::map<int, Song> week_data;
    while (node)
    { 
	myhtml_tag_id_t tag_id = myhtml_node_tag_id(node);
	if (tag_id == MyHTML_TAG_TBODY) {
	    myhtml_collection_t* tr_nodes = myhtml_get_nodes_by_name_in_scope(tree, NULL, node, "tr", 2, NULL);
	    // First column is just the labels
	    for (int i = 1; i < tr_nodes->length; ++i) {
		Song song_info = scrape_tr_nodes(tree, tr_nodes->list[i]);
		week_data.insert(std::pair<int, Song>(song_info.rank, song_info));
	    }
	    break;
	} 
	std::map<int, Song> data = extract_data(tree, myhtml_node_child(node));
        week_data.insert(data.begin(), data.end());
	node = myhtml_node_next(node);
    }
    return week_data;
}

std::vector<long> extract_song_ids(std::map<int, Song> week_data) 
{
    std::cout << "extract_song_ids\n";
    std::vector<long> song_ids;
    for (int i = 1; i < 101; ++i) {
	long id = week_data[i].song_id;
	song_ids.push_back(id);
    }
    return song_ids;
}

std::map<int, Song> GaonParser::parse(const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    std::map<int, Song> week_data = extract_data(tree, myhtml_node_child(node));
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return week_data;
}
