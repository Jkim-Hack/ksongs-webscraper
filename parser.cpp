#include "parser.hpp"
#include <string>
#include <iostream>
#include <curl/curl.h>

Parser::Parser() 
{
    curl = curl_easy_init();
}

Parser::~Parser() 
{
    curl_easy_cleanup(this->curl);
}

void Parser::load_info(SiteInfo site_info)
{
    site_infos.push_back(site_info);
}

// Prepare libcurl for get request
void Parser::prepare_handle(int index) 
{
    SiteInfo info = this->site_infos.at(index);
    if (curl) {
	std::cout << info.url.c_str() << std::endl;
	curl_easy_setopt(curl, CURLOPT_URL, info.url.c_str());
    }
}

void Parser::perform_all_handles()
{
}
