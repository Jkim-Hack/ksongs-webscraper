#include "parser.hpp"
#include "json.hpp"
#include <iostream>
#include <functional>
#include <map>
#include <myhtml/api.h>
#include <curl/curl.h>

void MelonParser::load_info(std::shared_ptr<SiteInfo> site_info, size_t max_dist_size)
{
    if (std::shared_ptr<MelonInfo> info = std::dynamic_pointer_cast<MelonInfo>(site_info)) {
	info->key_hash = std::stol(info->start_date) % max_dist_size;
	std::string start_year = info->start_date.substr(0, 4);
	int start_year_int = std::stoi(start_year.c_str());
	if (start_year_int >= 2017) info->gn_dp_id = "GN";
	else info->gn_dp_id = "DP";
	generate_url(info);
    } else {
	// TODO: Throw exception
    }
}

void MelonParser::generate_url(std::shared_ptr<SiteInfo> site_info)
{ 
    if (std::shared_ptr<MelonInfo> info = std::dynamic_pointer_cast<MelonInfo>(site_info)) {
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
}

std::shared_ptr<Song> MelonParser::scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) 
{
    std::shared_ptr<MelonSong> song_info(new MelonSong, [](MelonSong *song){delete song;});
    myhtml_collection_t* td_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tr_node, MyHTML_TAG_TD, NULL);
    for (int i = 0; i < td_nodes->length; ++i) {
	switch(i) {
	    case 0: {
			//Get title and id info
			myhtml_tree_node_t *input_node = myhtml_node_child(td_nodes->list[i]);
			input_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, input_node, MyHTML_TAG_INPUT, NULL)->list[0]; 
			if (input_node) {
			    std::map<std::string, std::string> attributes = get_node_attrs(input_node);
			    song_info->title = attributes.at("title");
			    song_info->song_id = std::stol(attributes.at("value"));
			}
			break;
		    }
	    case 1: {
			myhtml_tree_node_t *span_node = myhtml_node_child(td_nodes->list[i]);
			span_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, span_node, MyHTML_TAG_SPAN, NULL)->list[0];
			span_node = myhtml_node_child(span_node);
			if (span_node) {
			    song_info->rank = std::stoi(myhtml_node_text(span_node, NULL));
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

			    song_info->artist = myhtml_node_text(artist_node, NULL);
			    song_info->album = myhtml_node_text(album_node, NULL);	
			    song_info->artist_id = std::stol(artist_id);
			    song_info->album_id = std::stol(album_id);
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

std::vector<long> extract_song_ids(std::map<int, MelonSong*> week_data) 
{
    std::cout << "extract_song_ids\n";
    std::vector<long> song_ids;
    for (int i = 1; i < 101; ++i) {
	long id = week_data[i]->song_id;
	song_ids.push_back(id);
    }
    return song_ids;
}

void MelonParser::get_like_count(std::map<int, Song*>* week_data)
{
    std::cout << "get_like_count\n";
    std::string response_string;
    std::map<int, MelonSong*>* melon_week_data = (std::map<int, MelonSong*>*)week_data;
    if (curl) {
	std::vector<long> extracted_song_ids = extract_song_ids(*melon_week_data);
	std::string url = generate_like_count_url(extracted_song_ids);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &MelonParser::write);    
	curl_easy_perform(curl);

	// Parse json for like count
	nlohmann::json j = nlohmann::json::parse(response_string);
	for (auto& item : j.items().begin().value().items()) {
	    if (!item.key().empty()) {
		// TODO Cast here 
		long like_count = item.value()["SUMMCNT"];
		(*melon_week_data)[std::stoi(item.key()) + 1]->number_of_likes = like_count;
	    }
	}
    }
}

std::map<int, Song*> MelonParser::parse(const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);

    // function with the lambda calling our scrape function
    std::function<Song*(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> scrape_function = [=](myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) {
	return this->scrape_tr_nodes(tree, tr_node).get();
    };

    std::map<int, Song*> week_data = extract_data(tree, myhtml_node_child(node), 0, scrape_function);
    get_like_count(&week_data);
    
    myhtml_tree_destroy(tree);
    
    return week_data;
}

// Assume node is the top  level html node
void MelonParser::scrape_album(ID *id, myhtml_tree_t* tree, myhtml_tree_node_t* node, std::string target_title) 
{ 
    bool found_song_id = false;
    id->song_id = 0;
    id->album_id = 0;

    myhtml_collection_t *tbody_nodes = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_TBODY, NULL);
    if (tbody_nodes->length < 1) {
	// html is not valid
	std::cout << "INVALID HTML" << std::endl;
	// TODO: THROW EXCEPTION
	return;
    }
    for (size_t o = 0; o < tbody_nodes->length; ++o) {
	myhtml_tree_node_t *tbody_node = tbody_nodes->list[o];
	
	myhtml_collection_t *song_collection = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tbody_node, MyHTML_TAG_TR, NULL);
	for (size_t i = 0; i < song_collection->length; ++i) {
	    if (found_song_id) break;
	    // Getting song id
	    myhtml_collection_t *song_info_nodes = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, song_collection->list[i], false, "class", 5, "ellipsis", 8, NULL);
	    for (size_t j = 0; j < song_info_nodes->length; ++j) {
		if (j == 0) {
		    myhtml_tree_node_t *info_node = song_info_nodes->list[j];
		    myhtml_tree_node_t *child_node = myhtml_node_child(info_node);
		    myhtml_tree_node_t *title_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, child_node, MyHTML_TAG_A, NULL)->list[0];
		    if (title_node == NULL) {
			// Remember this gets ALL span nodes in the child, so we need the third span node
	    		title_node = myhtml_get_nodes_by_attribute_value(tree, NULL, child_node, false, "class", 5, "disabled", 8, NULL)->list[0];
		    }
		    std::string curr_title = myhtml_node_text(myhtml_node_child(title_node), NULL);
		    if (find_case_insensitive(target_title, curr_title)) {
			// we found our title
			myhtml_tree_node_t *input_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, song_collection->list[i], MyHTML_TAG_INPUT, NULL)->list[0];
			std::map<std::string, std::string> node_attrs = get_node_attrs(input_node);
			if (node_attrs.count("value")) {
			    // Song id found
			    std::string href_string = node_attrs["value"];
			    id->song_id = std::stol(href_string);
			    found_song_id = true;
			}
		    }
		} else if (j == 1 && found_song_id) {
		    // Getting artist id
		    myhtml_collection_t *a_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, song_info_nodes->list[j], MyHTML_TAG_A, NULL);
		    for (size_t k = 0; k < a_nodes->length; ++k) {
			std::map<std::string, std::string> node_attrs = get_node_attrs(a_nodes->list[k]);
			std::string href_string = node_attrs["href"];
			id->artist_ids.push_back(extract_ids_from_js(href_string)[0]);
		    }
		}
	    
	    }
	}
    }

    // Getting album id
    myhtml_collection_t *button_nodes = myhtml_get_nodes_by_attribute_key(tree, NULL, node, "data-album-no", 13, NULL);
    std::map<std::string, std::string> node_attrs = get_node_attrs(button_nodes->list[0]);
    id->album_id = std::stol(node_attrs["data-album-no"]);
}
