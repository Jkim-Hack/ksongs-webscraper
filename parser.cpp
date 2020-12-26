#include "parser.hpp"
#include <string>
#include <iostream>
#include <curl/curl.h>

Parser::Parser() 
{
    curl = curl_easy_init();
    this->myhtml = myhtml_create();
    myhtml_init(this->myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
}

Parser::~Parser() 
{
    curl_easy_cleanup(this->curl);
    myhtml_destroy(this->myhtml);
}

std::string Parser::extract_id(std::string text)
{
    size_t first = text.find_first_of("'");
    size_t last = text.find_last_of("'");
    return text.substr(first + 1, last-first-1);
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

size_t Parser::write(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

// Prepare libcurl for get request
std::string Parser::request_html(std::string url) 
{
    struct curl_slist *chunk = NULL;
    std::string response_string;
    if (curl) {
	chunk = curl_slist_append(chunk, "Cookie: PCID=16080592268936819734880");
	chunk = curl_slist_append(chunk, "PC_PCID=16080592268936819734880");
	chunk = curl_slist_append(chunk, "POC=MP10");
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write);    
	std::cout << url << std::endl;
	curl_easy_perform(curl);
    }
    return response_string.c_str();
}

std::map<int, Song*> Parser::extract_data(myhtml_tree_t *tree, myhtml_tree_node_t *node, std::function<Song*(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> &scrape_function)
{
    std::map<int, Song*> week_data;
    while (node)
    { 
	myhtml_tag_id_t tag_id = myhtml_node_tag_id(node);
	if (tag_id == MyHTML_TAG_TBODY) {
	    myhtml_collection_t* tr_nodes = myhtml_get_nodes_by_name_in_scope(tree, NULL, node, "tr", 2, NULL);
	    for (int i = 0; i < tr_nodes->length; ++i) {
		Song *song_info = scrape_function(tree, tr_nodes->list[i]);
		week_data.insert(std::pair<int, Song*>(song_info->rank, song_info));
	    }
	    break;
	}
	std::map<int, Song*> data = extract_data(tree, myhtml_node_child(node), scrape_function);
        week_data.insert(data.begin(), data.end());
	node = myhtml_node_next(node);
    }
    return week_data;
}
