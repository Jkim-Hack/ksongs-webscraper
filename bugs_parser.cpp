#include "parser.hpp"

void BugsParser::generate_url(std::shared_ptr<SiteInfo> info)
{
    info->url = "https://music.bugs.co.kr/chart/track/week/total?chartdate=" + info->start_date;
}

void BugsParser::load_info(std::shared_ptr<SiteInfo> info, size_t max_dist_size)
{
    info->key_hash = std::stol(info->start_date) % max_dist_size;
    generate_url(info);
}

std::vector<std::string> extract_artists(std::string artists_raw_value)
{
    std::vector<std::string> artists;
    
    size_t pos = artists_raw_value.find("OK", 0);
    while(pos != std::string::npos)
    {
	size_t bar_pos = artists_raw_value.find("||", pos+1);
	if (bar_pos != std::string::npos) {
	    std::string artist = artists_raw_value.substr(pos + 5, bar_pos - pos - 5);
	    std::cout << artist << std::endl;
	    artists.push_back(artist);
	}
	pos = artists_raw_value.find("OK",pos+1);
    }
    return artists;
}

std::shared_ptr<Song> BugsParser::scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)
{
    std::shared_ptr<BugsSong> song_info(new BugsSong, [](BugsSong *song) {delete song;});
    bool found_target = false;
    
    // Rank
    myhtml_tree_node_t *ranking_div_node = myhtml_get_nodes_by_attribute_value(tree, NULL, tr_node, false, "class", 5, "ranking", 7, NULL)->list[0];
    myhtml_tree_node_t *rank_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, ranking_div_node, MyHTML_TAG_STRONG, NULL)->list[0];
    rank_node = myhtml_node_child(rank_node);
    song_info->rank = std::stoi(myhtml_node_text(rank_node, NULL));

    myhtml_collection_t *p_nodes = myhtml_get_nodes_by_attribute_value(tree, NULL, tr_node, false, "class", 5, "title", 5, NULL);
    if (p_nodes->length > 0) {
	// Title
	myhtml_tree_node_t *title_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, p_nodes->list[0], MyHTML_TAG_A, NULL)->list[0];
	if (title_node == NULL) {
	    title_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, p_nodes->list[0], MyHTML_TAG_SPAN, NULL)->list[1];
	}
	std::string curr_title = myhtml_node_text(myhtml_node_child(title_node), NULL);
	std::cout <<  curr_title << std::endl;
	song_info->title = curr_title;
    }

    // Song id
    std::map<std::string, std::string> attrs = get_node_attrs(tr_node);
    song_info->song_id = std::stol(attrs["trackid"]);

    // Album id/name
    song_info->album_id = std::stol(attrs["albumid"]);
    myhtml_tree_node_t *album_node = myhtml_get_nodes_by_attribute_value(tree, NULL, tr_node, false, "class", 5, "album", 5, NULL)->list[0];
    album_node = myhtml_node_child(album_node);
    song_info->album = myhtml_node_text(album_node, NULL);

    // Artist id/name
    // More than one artist?
    if (attrs["multiartist"].compare("Y") != 0) {
	// No
	myhtml_tree_node_t *p_artist_node = myhtml_get_nodes_by_attribute_value(tree, NULL, tr_node, false, "class", 5, "artist", 6, NULL)->list[0];
	myhtml_tree_node_t *artist_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, p_artist_node, MyHTML_TAG_A, NULL)->list[0];
	if (artist_node == NULL) {
	    artist_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, p_artist_node, MyHTML_TAG_SPAN, NULL)->list[0];
	}
	artist_node = myhtml_node_child(artist_node);

	song_info->artists.push_back(myhtml_node_text(artist_node, NULL));
	song_info->artist_ids.push_back(std::stol(attrs["artistid"]));

    } else {
	// Yes
	myhtml_tree_node_t *artist_node = myhtml_get_nodes_by_attribute_value(tree, NULL, tr_node, false, "class", 5, "artist", 6, NULL)->list[0];
	myhtml_collection_t *artist_a_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, artist_node, MyHTML_TAG_A, NULL);
	std::map<std::string, std::string> attrs = get_node_attrs(artist_a_nodes->list[1]);
	std::vector<long> ids = extract_ids_from_js(attrs["onclick"]);
	for (auto &artist_id : ids) {
	    song_info->artist_ids.push_back(artist_id);
	}

	std::vector<std::string> artists = extract_artists(attrs["onclick"]);
	std::string first_artist = myhtml_node_text(myhtml_node_child(artist_a_nodes->list[1]), NULL);
	first_artist = remove_junk_spaces(first_artist);
	song_info->artists.push_back(first_artist);
	for (auto &artist : artists) {
	    song_info->artists.push_back(remove_junk_spaces(artist));
	}
    }
    std::cout << song_info->song_id << " : " << song_info->album_id << " : " << song_info->artist_ids.size() << std::endl; 
    return song_info;
}

std::map<int, std::shared_ptr<Song>> BugsParser::parse(const char* html_buffer)
{
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    std::function<std::shared_ptr<Song>(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> scrape_function = [=](myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) {
	return this->scrape_tr_nodes(tree, tr_node);
    };
    std::map<int, std::shared_ptr<Song>> week_data = extract_data(tree, myhtml_node_child(node), 0, scrape_function);
    
    myhtml_tree_destroy(tree); 
    return week_data;
}

void BugsParser::scrape_album(ID *ids, myhtml_tree_t* tree, myhtml_tree_node_t* node, std::string target_title)
{
    std::cout << __func__ << std::endl;
    
    std::cout << "Target: " << target_title << std::endl;

    // Getting parent table
    std::string table_attr = "list trackList byAlbum";
    myhtml_collection_t *table_nodes = myhtml_get_nodes_by_attribute_value(tree, NULL, node, false, "class", 5, table_attr.c_str(), table_attr.size(), NULL);
    
    if (table_nodes->length < 1) {
	std::cout << "INVALID HTML" << std::endl;
	return;
    }

    // Get inner tbody
    myhtml_collection_t *tbody_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, table_nodes->list[0], MyHTML_TAG_TBODY, NULL);
   
    bool found_target = false;
    for (size_t j = 0; j < tbody_nodes->length; ++j) {
	if (found_target) break;
	myhtml_tree_node_t *tbody_node = tbody_nodes->list[j];
	// Get inner tr nodes
	myhtml_collection_t *tr_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tbody_node, MyHTML_TAG_TR, NULL);
	std::cout << "here" << std::endl;
	for (int i = 0; i < tr_nodes->length; ++i) {
	    myhtml_collection_t *p_nodes = myhtml_get_nodes_by_attribute_value(tree, NULL, tr_nodes->list[i], false, "class", 5, "title", 5, NULL);
	    if (p_nodes->length > 0) {
		myhtml_tree_node_t *title_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, p_nodes->list[0], MyHTML_TAG_A, NULL)->list[0];
		if (title_node == NULL) {
		    title_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, p_nodes->list[0], MyHTML_TAG_SPAN, NULL)->list[1];
		}
		std::string curr_title = myhtml_node_text(myhtml_node_child(title_node), NULL);
		std::cout <<  curr_title << std::endl;
		if (find_case_insensitive(target_title, curr_title)) {
		    found_target = true;
		    std::cout << target_title << std::endl;
		    std::map<std::string, std::string> attrs = get_node_attrs(tr_nodes->list[i]);
		    ids->song_id = std::stol(attrs["trackid"]);
		    ids->album_id = std::stol(attrs["albumid"]);

		    // More than one artist?
		    if (attrs["multiartist"].compare("Y") != 0) {
			// No
			ids->artist_ids.push_back(std::stol(attrs["artistid"]));
		    } else {
			// Yes
			myhtml_tree_node_t *artist_node = myhtml_get_nodes_by_attribute_value(tree, NULL, tr_nodes->list[i], false, "class", 5, "artist", 6, NULL)->list[0];
			myhtml_collection_t *artist_a_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, artist_node, MyHTML_TAG_A, NULL);
			std::map<std::string, std::string> attrs = get_node_attrs(artist_a_nodes->list[1]);
			std::vector<long> artist_ids_long = extract_ids_from_js(attrs["onclick"]);
			for (auto &artist_id : artist_ids_long) {
			    ids->artist_ids.push_back(artist_id);
			}
		    }
		    break;
		}
	    }
	}
    }    
}
