#include "melon_parser.hpp"
#include "json.hpp"
#include <iostream>
#include <functional>
#include <map>
#include <myhtml/api.h>
#include <curl/curl.h>

void MelonParser::load_info(MelonInfo *info)
{
    std::string start_year = info->start_date.substr(0, 4);
    int start_year_int = std::stoi(start_year.c_str());
    if (start_year_int >= 2017) info->gn_dp_id = "GN";
    else info->gn_dp_id = "DP";
    generate_url(info);
}

void MelonParser::generate_url(MelonInfo *info)
{ 
    std::string start_year = info->start_date.substr(0, 4);
    std::string end_year = info->end_date.substr(0, 4);
    int end_year_int = std::stoi(end_year);
    std::string url;
    std::string month = info->start_date.substr(4, 2);
    if (end_year_int == 2020) {
	url = "https://www.melon.com/chart/week/index.htm?chartType=WE&age=2020&year=2020&mon=" + month + "&day=" + info->start_date + "%5E" + info->end_date + "&classCd=GN0000&startDay=" + info->start_date + "&endDay=" + info->end_date + "&moved=Y";
    } else if (end_year_int < 2020 && end_year_int >= 2010) {
	url = "https://www.melon.com/chart/search/list.htm?chartType=WE&age=2010&year=" + start_year + "&mon=" + month + "&day=" + info->start_date +"%5E"+ info->end_date + "&classCd="+ info->gn_dp_id +"0000&startDay="+info->start_date+"&endDay="+info->end_date+"&moved=Y";
    } else {
	// TODO: Throw exception
    }
}

Song MelonParser::scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) 
{
    Song song_info;
    myhtml_collection_t* td_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tr_node, MyHTML_TAG_TD, NULL);
    for (int i = 0; i < td_nodes->length; ++i) {
	switch(i) {
	    case 0: {
			//Get title and id info
			myhtml_tree_node_t *input_node = myhtml_node_child(td_nodes->list[i]);
			input_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, input_node, MyHTML_TAG_INPUT, NULL)->list[0]; 
			if (input_node) {
			    std::map<std::string, std::string> attributes = get_node_attrs(input_node);
			    song_info.title = attributes.at("title");
			    song_info.song_id = std::stol(attributes.at("value"));
			}
			break;
		    }
	    case 1: {
			myhtml_tree_node_t *span_node = myhtml_node_child(td_nodes->list[i]);
			span_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, span_node, MyHTML_TAG_SPAN, NULL)->list[0];
			span_node = myhtml_node_child(span_node);
			if (span_node) {
			    song_info.rank = std::stoi(myhtml_node_text(span_node, NULL));
			}
			break;
		    }
	    case 3: {
			myhtml_tree_node_t *div_node = myhtml_get_nodes_by_attribute_value(tree, NULL, td_nodes->list[i], true, "class", 5, "wrap_song_info", 14, NULL)->list[0];
			if (div_node) {
			    myhtml_collection_t* sub_div_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, div_node, MyHTML_TAG_DIV, NULL);
			    
			    myhtml_tree_node_t* second_div_node = sub_div_nodes->list[1];
			    myhtml_collection_t* second_div_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, second_div_node, MyHTML_TAG_DIV, NULL);

			    myhtml_collection_t* artist_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, second_div_nodes->list[0], MyHTML_TAG_A, NULL);
			    std::string artist_id; 
			    if (artist_nodes->length < 1) {
				artist_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, second_div_nodes->list[0], MyHTML_TAG_SPAN, NULL);
				artist_id = "-";
			    } else {
				artist_id = extract_id(get_node_attrs(artist_nodes->list[0])["href"]);
			    }
			    myhtml_tree_node_t* artist_node = myhtml_node_child(artist_nodes->list[0]);

			    myhtml_tree_node_t* album_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, second_div_nodes->list[1], MyHTML_TAG_A, NULL)->list[0];
			    std::string album_id = extract_id(get_node_attrs(album_node)["href"]);
			    album_node = myhtml_node_child(album_node);

			    song_info.artist = myhtml_node_text(artist_node, NULL);
			    song_info.album = myhtml_node_text(album_node, NULL);	
			    song_info.artist_id = artist_id;
			    song_info.album_id = album_id;
			}
			break;
		    }
	}
    }
    return song_info;
}


std::string MelonParser::generate_like_count_url(std::vector<long> song_ids)
{
    std::string url = "https://www.melon.com/commonlike/getSongLike.json?contsIds="; 
    for (auto id : song_ids) {
	url.append(std::to_string(id));
	url.append("%2C");
    }
    url.erase(url.length() - 3, 3);
    return url;
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

void MelonParser::get_like_count(std::map<int, Song>* week_data)
{
    std::cout << "get_like_count\n";
    std::string response_string;
    if (curl) {
	std::vector<long> extracted_song_ids = extract_song_ids(*week_data);
	std::string url = generate_like_count_url(extracted_song_ids);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &MelonParser::write);    
	curl_easy_perform(curl);
	nlohmann::json j = nlohmann::json::parse(response_string);
	for (auto& item : j.items().begin().value().items()) {
	    if (!item.key().empty()) {
		long like_count = item.value()["SUMMCNT"];
		(*week_data)[std::stoi(item.key()) + 1].number_of_likes = like_count;
	    }
	}
    }
}

std::map<int, Song> MelonParser::parse(const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);

    // function with the lambda calling our scrape function
    std::function<Song(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> scrape_function = [=](myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) {
	return this->scrape_tr_nodes(tree, tr_node);
    };

    std::map<int, Song> week_data = extract_data(tree, myhtml_node_child(node), scrape_function);
    get_like_count(&week_data);
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return week_data;
}
