#include "parser.hpp"
#include <iostream>
#include <curl/curl.h>
#include <algorithm>

void GaonParser::generate_url(std::shared_ptr<SiteInfo> site_info) 
{
    // Cast to GaonInfo pointer
    if (std::shared_ptr<GaonInfo> info = std::dynamic_pointer_cast<GaonInfo>(site_info)) {
	// Retrive url based on the Gaon site type
	std::string url;
	if (info->type == DIGITAL) {
	    url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=ALL&targetTime=" + std::to_string(info->week) + "&hitYear=" + std::to_string(info->year) + "&termGbn=week";
	} else if (info->type == DOWNLOAD) {
	    url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=S1020&targetTime=" + std::to_string(info->week) + "&hitYear=" + std::to_string(info->year) + "&termGbn=week";
	} else if (info->type == STREAMING) { 
	    url = "http://gaonchart.co.kr/main/section/chart/online.gaon?nationGbn=T&serviceGbn=S1040&targetTime=" + std::to_string(info->week) + "&hitYear=" + std::to_string(info->year) + "&termGbn=week";
	} else {
	    // The type was missing
	    // TODO: Throw exception type missing
	}
	info->url = url;
    } else {
	// TODO: Exception pointer type mismatch
    }
}

// Generate song irl based on the song id and the site we want
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

// Scrape the melon album for the song and its info
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
    if (html.length() < 4) {
	std::cout << "ERROR ERROR ERROR ERROR" << std::endl;
	std::cout << html << std::endl;
    }
    MelonParser::scrape_album(&melon_ids, tree, node, curr_song->title);

    std::cout << melon_ids.song_id << " : " << melon_ids.album_id << " : " << melon_ids.artist_ids.size() << std::endl; 

    // Finally add to the song's final output map
    curr_song->site_ids.insert(std::pair<SITE, ID>(MELON, melon_ids));
    
    myhtml_tree_destroy(tree); 
}

// Scrape the bugs album for the song and its info
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
    curr_song->site_ids.insert(std::pair<SITE, ID>(BUGS, bugs_ids));
    
    myhtml_tree_destroy(tree); 
}

// Scrape the genie album for the song and its info
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

// Gets all the song ids from melon, bugs, and genie with the list nodes from the gaon song
std::map<SITE, long> GaonParser::get_all_song_ids(myhtml_tree_t* tree, myhtml_collection_t *li_nodes)
{
    std::cout << __func__ << std::endl;
    std::map<SITE, long> all_song_ids;

    // Enumerate over the SITE enums to Genie
    for (int i = MELON; i <= GENIE; ++i) {
	myhtml_tree_node_t *a_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, li_nodes->list[i], MyHTML_TAG_A, NULL)->list[0];
	if (a_node != NULL) {
	    std::map<std::string, std::string> node_attrs = get_node_attrs(a_node);
	    if (node_attrs.find("href") != node_attrs.end()) {
		std::string href_string = node_attrs["href"];
		SITE site = (SITE)i;
		all_song_ids[site] = extract_ids_from_js(href_string)[1];
	    }
	}
    }

    return all_song_ids;
}

// Extracts all the ids from melon, bugs, and genie
void GaonParser::extract_all_ids(std::shared_ptr<GaonSong> curr_song, myhtml_tree_t* tree, myhtml_collection_t *li_nodes)
{
    std::cout << __func__ << std::endl;
    std::map<SITE, long> song_ids = get_all_song_ids(tree, li_nodes);
    // iterate through all the sites and their song ids
    for (auto const& [site, song_id] : song_ids) {
	// Generate the song url
	std::string url = generate_song_url(song_id, site);
	std::string html = request_html(url);
	
	// extract the redirect url from the html
	size_t left = html.find_first_of("HREF=");
	size_t right = html.find_first_of(">", left);

	// Make sure that there is a url in the html
	if (left != std::string::npos && right != std::string::npos) {
	    // remove all unecessary spaces and qutoes
	    url = html.substr(left+6, right - (left+6));	
	    url.erase(std::remove(url.begin(), url.end(), '\t'), url.end());
	    url.erase(std::remove(url.begin(), url.end(), '\"'), url.end());
	    
	    size_t http_pos = url.find_first_of("p");
	    std::string site_html;
	    switch(site) {
		case MELON:
		    {
			// Melon doesn't have an 's' at the end of the 'https' substring
			url.insert(http_pos+1, "s");
			site_html = request_html(url);
			scrape_melon_song(curr_song, site_html);
			// If the song failed to scrape then print to error file
			if (curr_song->site_ids[MELON].song_id == 0) 
			    err_output << "ERROR: " << url  << " Target: " << curr_song->title << std::endl;
			break;
		    }
		case BUGS:
		    {
			// Same as the melon url 
			url.insert(http_pos+1, "s");
			site_html = request_html(url);
			scrape_bugs_song(curr_song, site_html);
			// Same as melon error printing
			if (curr_song->site_ids[BUGS].song_id == 0) 
			    err_output << "ERROR: " << url  << " Target: " << curr_song->title << std::endl;
			break;
		    }
		case GENIE:
		    {
			// Genie has the correct format so no need to insert an 's'
			site_html = request_html(url);
			scrape_genie_song(curr_song, site_html);
			// Same as melon and bugs with error printing
			if (curr_song->site_ids[GENIE].song_id == 0) 
			    err_output << "ERROR: " << url  << " Target: " << curr_song->title << std::endl;
			break;
		    }
		default:
		    // TODO: ERROR: we should not be getting anything else
		    break;
	    }
	    curr_song->site_htmls.insert(std::pair<SITE, std::string>(site, site_html));
	} else {
	    std::cout << html << std::endl;
	    // TODO: Throw exception
	}
    }
}

// Sets up the site info and the hash code for thread distribution
void GaonParser::load_info(std::shared_ptr<SiteInfo> site_info, size_t max_dist_size)
{
    if (std::shared_ptr<GaonInfo> info = std::dynamic_pointer_cast<GaonInfo>(site_info)) {
	info->key_hash = (info->week * info->year) % max_dist_size;
	generate_url(info);
    } else {
	// TODO: THROW EXCEPTION
    }
}


// Extract the date from the Gaon site
void GaonParser::extract_dates(std::shared_ptr<SiteInfo> site_info, const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t* doc_node = myhtml_tree_get_document(tree);
    myhtml_tree_node_t* root =  myhtml_node_child(doc_node);
    
    myhtml_tree_node_t* selection_node = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_SELECT, NULL)->list[0];
    if (selection_node != NULL) {
	myhtml_collection_t *option_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, selection_node, MyHTML_TAG_OPTION, NULL);
	if (option_nodes->length > 0) {
	    if (std::shared_ptr<GaonInfo> info = std::dynamic_pointer_cast<GaonInfo>(site_info)) {
		
		info->start_date = "";
		info->end_date = "";
		int week = info->week;
		int year = info->year;

		// Get the date from looking at the option node
		std::string year_week = std::to_string(year) + std::to_string(week);
		myhtml_collection_t *date_nodes = myhtml_get_nodes_by_attribute_value(tree, NULL, selection_node, false, "value", 5, year_week.c_str(), year_week.length(), NULL);
		if (date_nodes->length > 0) {
		    // Now get the select node for our date and load in the siteinfo
		    myhtml_tree_node_t *selected_node = date_nodes->list[0];
		    if (selected_node != NULL) {
			myhtml_tree_node_t *text_node = myhtml_node_child(selected_node);
			std::string dates = myhtml_node_text(text_node, NULL);

			// separate the start and end and remove unnecessary characters
			size_t delim_pos = dates.find("~");
			std::string start = dates.substr(0, delim_pos);
			std::string end = dates.substr(delim_pos+1, dates.length() - delim_pos);
			start.erase(std::remove(start.begin(), start.end(), '.'), start.end());
			end.erase(std::remove(end.begin(), end.end(), '.'), end.end());

			// load in the info
			info->start_date = start;
			info->end_date = end;
		    }
		}
	    }
	}
    } else {
	std::cout << "ERROR: INVALID HTML" << std::endl;
    }
    myhtml_tree_destroy(tree);
}

// get rank info
void GaonParser::extract_rank(std::shared_ptr<GaonSong> song_info, myhtml_tree_t* tree, myhtml_tree_node_t *td_node)
{
    myhtml_tree_node_t *span_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_node, MyHTML_TAG_SPAN, NULL)->list[0];
    myhtml_tree_node_t *inner_text; 
    if (span_node == NULL) {
	inner_text = myhtml_node_child(td_node);
    } else {
	inner_text = myhtml_node_child(span_node);
    }
    song_info->rank = std::stoi(myhtml_node_text(inner_text, NULL));
}

// get the title, artists, and the album, along with their ids
void GaonParser::extract_title_artists_album(std::shared_ptr<GaonSong> song_info, myhtml_tree_t* tree, myhtml_tree_node_t* td_node)
{
    myhtml_collection_t *p_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_node, MyHTML_TAG_P, NULL);
    if (p_nodes->length > 1) {
	std::map<std::string, std::string> title_attrs = Parser::get_node_attrs(p_nodes->list[0]);
	std::map<std::string, std::string> artist_attrs = Parser::get_node_attrs(p_nodes->list[1]);	
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
    }
}

// get the gaon index
void GaonParser::extract_gaon_index(std::shared_ptr<GaonSong> song_info, myhtml_tree_t* tree, myhtml_tree_node_t* td_node)
{
    if (td_node) {
	std::map<std::string, std::string> attr = Parser::get_node_attrs(td_node);
	if (attr.size() > 0 && attr.find("class") != attr.end() && attr["class"].compare("count") == 0) {
	    myhtml_tree_node_t *p_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_node, MyHTML_TAG_P, NULL)->list[0]; 
	    std::string index = myhtml_node_text(myhtml_node_child(p_node), NULL);
	    index.erase(std::remove(index.begin(), index.end(), ','), index.end());
	    song_info->gaon_index = std::stol(index);
	} else {
	    song_info->gaon_index = 0;
	}
    }
}

// get all the song ids from melon, bugs, genie
void GaonParser::extract_song_ids(std::shared_ptr<GaonSong> song_info, myhtml_tree_t* tree, myhtml_tree_node_t* td_node)
{
    myhtml_collection_t *li_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_node, MyHTML_TAG_LI, NULL);
    if (li_nodes->length > 2) {
	extract_all_ids(song_info, tree, li_nodes);
    }
}

std::shared_ptr<Song> GaonParser::scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) 
{
    // Create a new song per tr node
    std::shared_ptr<GaonSong> song_info(new GaonSong, [](GaonSong *song) {delete song;});
    myhtml_collection_t* td_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tr_node, MyHTML_TAG_TD, NULL);
    
    // Each td node will have info we need, the info will be on the 0th, 3rd, 4th, 6th (or 7th depending on the date of
    // the chart). 
    for (int i = 0; i < td_nodes->length; ++i) {
	switch(i) {
	    case 0: 
		{   
		    extract_rank(song_info, tree, td_nodes->list[i]);
		    break;
		}
	    case 3: 
		{
		    extract_title_artists_album(song_info, tree, td_nodes->list[i]);
		    break;
		}
	    case 4:
		{
		    extract_gaon_index(song_info, tree, td_nodes->list[i]);
		    break;
		}
	    case 6:
		{
		    extract_song_ids(song_info, tree, td_nodes->list[i]);
		    break;
		}
	    case 7:
		{
		    extract_song_ids(song_info, tree, td_nodes->list[i]);
		    break;
		}
	}
    }
    return song_info;
}

// Parse through the site
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
