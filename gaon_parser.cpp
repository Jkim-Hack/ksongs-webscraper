#include "parser.hpp"
#include <iostream>
#include <curl/curl.h>
#include <algorithm>

void GaonParser::generate_url(std::shared_ptr<SiteInfo> site_info) 
{
    if (std::shared_ptr<GaonInfo> info = std::dynamic_pointer_cast<GaonInfo>(site_info)) {
	std::string url;
	if (info->type == DIGITAL) {
	    url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=ALL&targetTime=" + std::to_string(info->week) + "&hitYear=" + std::to_string(info->year) + "&termGbn=week";
	} else if (info->type == DOWNLOAD) {
	    url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=S1020&targetTime=" + std::to_string(info->week) + "&hitYear=" + std::to_string(info->year) + "&termGbn=week";
	} else if (info->type == STREAMING) { 
	    url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=S1040&targetTime=" + std::to_string(info->week) + "&hitYear=" + std::to_string(info->year) + "&termGbn=week";
	} else {
	    // TODO: Exception
	}
	info->url = url;
    } else {
	// TODO: Exception
    }
}

std::string GaonParser::generate_song_url(long song_id, enum SITE site)
{
    int company;
    switch (site) {
	case MELON:
	    company = 3715;
	    break;
	case BUGS:
	    company = 1594;
	    break;
	case GENIE:
	    company = 2407;
	    break;
	default:
	    company = 3715;
    }
    return "http://gaonchart.co.kr/main/section/chart/ReturnUrl.gaon?serviceGbn=ALL&seq_company=" + std::to_string(company) + "&seq_mom=" + std::to_string(song_id);
}

void GaonParser::scrape_melon_song(std::shared_ptr<GaonSong> curr_song, std::string html)
{
    std::cout << __func__ << std::endl;
    ID melon_ids;
   
    melon_ids.song_id = 0;
    melon_ids.album_id = 0;

    // Setup parser
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    const char* html_buffer = html.c_str();
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    MelonParser::scrape_album(&melon_ids, tree, node, curr_song->title);

    std::cout << melon_ids.song_id << " : " << melon_ids.album_id << " : " << melon_ids.artist_ids.size() << std::endl; 

    // Finally add to the song's final output map
    curr_song->site_ids.insert(std::pair<SITE, ID>(MELON, melon_ids));
    
    myhtml_tree_destroy(tree); 
}

void GaonParser::scrape_bugs_song(std::shared_ptr<GaonSong> curr_song, std::string html)
{
    std::cout << __func__ << std::endl;
    ID bugs_ids;

    bugs_ids.song_id = 0;
    bugs_ids.album_id = 0;

    // Setup parser
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    const char* html_buffer = html.c_str();
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    BugsParser::scrape_album(&bugs_ids, tree, node, curr_song->title);

    std::cout << bugs_ids.song_id << " : " << bugs_ids.album_id << " : " << bugs_ids.artist_ids.size() << std::endl; 

    // Finally add to the song's final output map
    curr_song->site_ids.insert(std::pair<SITE, ID>(MELON, bugs_ids));
    
    myhtml_tree_destroy(tree); 
}

void GaonParser::scrape_genie_song(std::shared_ptr<GaonSong> curr_song, std::string html)
{
    std::cout << __func__ << curr_song->title<< std::endl;
    ID genie_ids;

    genie_ids.album_id = 0;
    genie_ids.song_id = 0;

    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    const char* html_buffer = html.c_str();
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);

    GenieParser::scrape_album(&genie_ids, tree, node, curr_song->title);
    
    std::cout << genie_ids.song_id << " : " << genie_ids.album_id << " : " << genie_ids.artist_ids.size() << std::endl; 
    // Finally add to the final output
    curr_song->site_ids.insert(std::pair<SITE, ID>(GENIE, genie_ids));
    
    myhtml_tree_destroy(tree); 
}

std::map<SITE, long> GaonParser::get_all_song_ids(myhtml_collection_t *li_nodes)
{
    std::cout << __func__ << std::endl;
    std::map<SITE, long> all_song_ids;

    // Enumerate over the SITE enums to Genie
    for (int i = MELON; i <= GENIE; ++i) {
	myhtml_tree_node_t *a_node = myhtml_node_child(li_nodes->list[i]);
	std::map<std::string, std::string> node_attrs = get_node_attrs(a_node);
	std::string href_string = node_attrs["href"];
	SITE site = (SITE)i;
	all_song_ids[site] = extract_ids_from_js(href_string)[1];
    }

    return all_song_ids;
}

void GaonParser::extract_all_ids(std::shared_ptr<GaonSong> curr_song, myhtml_collection_t *li_nodes)
{
    std::cout << __func__ << std::endl;
    std::map<SITE, long> song_ids = get_all_song_ids(li_nodes);
    for (auto const& [site, song_id] : song_ids) {
	std::string url = generate_song_url(song_id, site);
	std::string html = request_html(url);
	size_t left = html.find_first_of("HREF=");
	size_t right = html.find_first_of(">", left);

	if (left != std::string::npos && right != std::string::npos) {
	    // TODO: Throw exception
	    size_t http_pos = url.find_first_of("p");
	    url = html.substr(left+6, right - (left+6));	
	    url.erase(std::remove(url.begin(), url.end(), '\t'), url.end());
	    url.erase(std::remove(url.begin(), url.end(), '\"'), url.end());
	    
	    switch(site) {
		case MELON:
		    {
			url.insert(http_pos+1, "s");
			scrape_melon_song(curr_song, request_html(url));
			break;
		    }
		case BUGS:
		    {
			url.insert(http_pos+1, "s");
			scrape_bugs_song(curr_song, request_html(url));
			break;
		    }
		case GENIE:
		    {
			scrape_genie_song(curr_song, request_html(url));
			break;
		    }
		default:
		    // TODO: ERROR
		    break;
	    }
	} else {
	    std::cout << html << std::endl;
	}
    }
}

void GaonParser::load_info(std::shared_ptr<SiteInfo> site_info, size_t max_dist_size)
{
    if (std::shared_ptr<GaonInfo> info = std::dynamic_pointer_cast<GaonInfo>(site_info)) {
	info->key_hash = (info->week * info->year) % max_dist_size;
	generate_url(info);
    } else {
	// TODO: THROW EXCEPTION
    }
}

void GaonParser::extract_dates(std::shared_ptr<SiteInfo> site_info, const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t* doc_node = myhtml_tree_get_document(tree);
    myhtml_tree_node_t* root =  myhtml_node_child(doc_node);
    
    myhtml_tree_node_t* selection_node = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_SELECT, NULL)->list[0];

    myhtml_collection_t *option_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, selection_node, MyHTML_TAG_OPTION, NULL);
    std::cout << option_nodes->length << std::endl;
    if (std::shared_ptr<GaonInfo> info = std::dynamic_pointer_cast<GaonInfo>(site_info)) {
	int week = info->week;
	int year = info->year;

	int total_from_start;
	total_from_start = ((year - 2010) * 52) + week - 1; 

	int select_node_index = option_nodes->length - total_from_start;

	myhtml_tree_node_t *selected_node = option_nodes->list[select_node_index];
	std::cout << "Getting date " << info->url << "\n";
	
	myhtml_tree_node_t *text_node = myhtml_node_child(selected_node);
	std::string dates = myhtml_node_text(text_node, NULL);

	size_t delim_pos = dates.find("~");
	std::string start = dates.substr(0, delim_pos);
	std::string end = dates.substr(delim_pos+1, dates.length() - delim_pos);

	start.erase(std::remove(start.begin(), start.end(), '.'), start.end());
	end.erase(std::remove(end.begin(), end.end(), '.'), end.end());

	info->start_date = start;
	info->end_date = end;
    }
    myhtml_tree_destroy(tree);
}

std::shared_ptr<Song> GaonParser::scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) 
{
    std::shared_ptr<GaonSong> song_info(new GaonSong, [](GaonSong *song) {delete song;});
    myhtml_collection_t* td_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tr_node, MyHTML_TAG_TD, NULL);
    for (int i = 0; i < td_nodes->length; ++i) {
	switch(i) {
	    case 0: 
		{    
		    //Get title and id info
		    myhtml_tree_node_t *span_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_SPAN, NULL)->list[0];
		    myhtml_tree_node_t *inner_text; 
		    if (span_node == NULL) {
			inner_text = myhtml_node_child(td_nodes->list[i]);
		    } else {
			inner_text = myhtml_node_child(span_node);
		    }
		    song_info->rank = std::stoi(myhtml_node_text(inner_text, NULL));
		    break;
		}
	    case 3: 
		{
		    myhtml_collection_t *p_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_P, NULL);
		    std::map<std::string, std::string> title_attrs = get_node_attrs(p_nodes->list[0]);
		    std::map<std::string, std::string> artist_attrs = get_node_attrs(p_nodes->list[1]);	
		    std::string art_alb = artist_attrs["title"];

		    size_t delim_pos = art_alb.find("|");
		    if (delim_pos != std::string::npos) {
			std::string artists = art_alb.substr(0, delim_pos);
			std::string comma_delim = " , ";
			if (artists.find(comma_delim) != std::string::npos) {
			    size_t comma_pos = 0;
			    std::string artist;
			    while ((comma_pos = artists.find(comma_delim)) != std::string::npos) {
				artist = artists.substr(0, comma_pos);
				song_info->artists.push_back(artist);
				artists.erase(0, comma_pos + comma_delim.length());
			    }
			} else {
			    song_info->artists.push_back(artists);
			}
			song_info->album = art_alb.substr(delim_pos+2, art_alb.length() - delim_pos);
		    } else {
			std::cout << "Artists and album not found\n";
		    }
		    song_info->title = title_attrs["title"];
		    break;
		}
	    case 4:
		{
		    std::map<std::string, std::string> attr = get_node_attrs(td_nodes->list[i]);
		    if (attr.size() > 0 && attr["class"].compare("count") == 0) {
			myhtml_tree_node_t *p_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_P, NULL)->list[0]; 
			std::string index = myhtml_node_text(myhtml_node_child(p_node), NULL);
			index.erase(std::remove(index.begin(), index.end(), ','), index.end());
			song_info->gaon_index = std::stol(index);
		    	std::cout << song_info->gaon_index << std::endl;
		    } else {
			song_info->gaon_index = 0;
		    }
		    break;
		}
	    case 6:
		{
		    myhtml_collection_t *li_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_LI, NULL);
		    if (li_nodes->length > 0) {
			extract_all_ids(song_info, li_nodes);
		    }
		    break;
		}
	    case 7:
		{
		    myhtml_collection_t *li_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_LI, NULL);
		    if (li_nodes->length > 0) {
			extract_all_ids(song_info, li_nodes);
		    }
		    break;
		}
	}
    }
    return song_info;
}

std::map<int, std::shared_ptr<Song>> GaonParser::parse(const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    std::function<std::shared_ptr<Song>(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> scrape_function = [=](myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) {
	return this->scrape_tr_nodes(tree, tr_node);
    };
    std::map<int, std::shared_ptr<Song>> week_data = extract_data(tree, myhtml_node_child(node), 1, scrape_function);
    
    myhtml_tree_destroy(tree); 
    return week_data;
}
