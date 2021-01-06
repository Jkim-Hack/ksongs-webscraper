#include "parser.hpp"

void GenieParser::generate_url(std::shared_ptr<SiteInfo> info)
{
    info->url = "https://www.genie.co.kr/chart/top200?ditc=W&rtm=N&ymd=" + info->start_date;
}

void GenieParser::load_info(std::shared_ptr<SiteInfo> info, size_t max_dist_size)
{
    info->key_hash = std::stol(info->start_date) % max_dist_size;
    generate_url(info);
}

std::shared_ptr<Song> GenieParser::scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)
{
    std::shared_ptr<GenieSong> song_info(new GenieSong, [](GenieSong *song) {delete song;});

    myhtml_tree_node_t* td_node_info = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, tr_node, true, "class", 5, "info", 4, NULL)->list[0];
    myhtml_collection_t *a_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_node_info, MyHTML_TAG_A, NULL);
    
    myhtml_tree_node_t* title_node_info = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, tr_node, true, "class", 5, "info", 4, NULL)->list[0];

    myhtml_tree_node_t* artist_node_info = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, tr_node, true, "class", 5, "info", 4, NULL)->list[0];

    myhtml_tree_node_t* album_node_info = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, tr_node, true, "class", 5, "info", 4, NULL)->list[0];
    
    // Getting song id
    std::map<std::string, std::string> attrs = get_node_attrs(title_node_info);
    attrs = get_node_attrs(title_node_info);
    std::string song_id = attrs["onclick"];
    song_info->song_id = extract_ids_from_js(song_id)[0];
    song_info->title = myhtml_node_text(myhtml_node_child(title_node_info), NULL);

    attrs = get_node_attrs(artist_node_info);
    std::string artist_id = attrs["onclick"];
    song_info->artist_id = extract_ids_from_js(artist_id)[0];
    song_info->artists.push_back(myhtml_node_text(myhtml_node_child(title_node_info), NULL));
    
    attrs = get_node_attrs(album_node_info);
    std::string album_id = attrs["onclick"];
    song_info->album_id = extract_ids_from_js(album_id)[0];
    song_info->album = myhtml_node_text(myhtml_node_child(album_node_info), NULL);
    return song_info;
}

std::map<int, std::shared_ptr<Song>> GenieParser::parse(const char* html_buffer)
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

void GenieParser::scrape_album(ID *ids, myhtml_tree_t* tree, myhtml_tree_node_t* node, std::string target_title) {
    // Getting album id
    myhtml_collection_t *album_nodes = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, node, true, "onclick", 7, "fnPlayAlbum", 11, NULL);

    if (album_nodes->length < 1) {
	std::cout << "Failed to scrape Album section" << std::endl;
    	return;
    } else {
	myhtml_tree_node_t *album_node = album_nodes->list[0];
	std::map<std::string, std::string> attrs = get_node_attrs(album_node);
	std::string onclick_attr = attrs["onclick"];
	ids->album_id = extract_ids_from_js(onclick_attr)[0];
    }

    std::cout << "Target: " << target_title << std::endl;

    myhtml_collection_t *tbody_nodes = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_TBODY, NULL);
    for (size_t j = 0; j < tbody_nodes->length; ++j) {
	myhtml_tree_node_t *tbody_node = tbody_nodes->list[j];
	if (tbody_node == NULL) {
	    std::cout << "Failed to scrape song/artist section" << std::endl;
	    return;
	}

	myhtml_collection_t *tr_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tbody_node, MyHTML_TAG_TR, NULL);

	for (int i = 0; i < tr_nodes->length; ++i) {
	    // Getting song id
	    std::map<std::string, std::string> attrs = get_node_attrs(tr_nodes->list[i]);
	    std::string song_id = attrs["songid"];
	    myhtml_tree_node_t *input_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tr_nodes->list[i], MyHTML_TAG_INPUT, NULL)->list[0];
	    if (input_node != NULL) {
		std::string curr_title = get_node_attrs(input_node)["title"];
		if (find_case_insensitive(target_title, curr_title)) {
		    std::cout << curr_title << std::endl;
		    ids->song_id = extract_ids_from_js(song_id)[0];
		    // Getting artist id
		    myhtml_tree_node_t *artist_node = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, tr_nodes->list[i], true, "onclick", 7, "fnViewArtist", 12, NULL)->list[0];
		    attrs = get_node_attrs(artist_node);
		    std::string onclick_attr = attrs["onclick"];
		    ids->artist_ids.push_back(extract_ids_from_js(onclick_attr)[0]);
		    break;
		}
	    }
	}
    }
}
