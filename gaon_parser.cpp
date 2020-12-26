#include "gaon_parser.hpp"
#include <iostream>
#include <curl/curl.h>
#include <algorithm>
#include <array>

void GaonParser::generate_url(GaonInfo *info) 
{
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

std::array<long, 2> extract_ids_from_js(std::string attr_string, std::string delim)
{
    std::array<long, 2> ids;
    size_t left = attr_string.find_first_of("(");
    size_t right = attr_string.find_first_of(")");
    std::string js_code = attr_string.substr(left + 1, right - 1);

    if (delim.length() == 0) {
	ids[0] = std::stol(js_code);
	ids[1] = 0;
    } else {
	size_t delim_pos = js_code.find(delim);
	ids[0] = std::stol(js_code.substr(1, delim_pos - 1));
	ids[1] = std::stol(js_code.substr(delim_pos + 1, js_code.length() - delim_pos - 2));
    }

    return ids;
}

void GaonParser::scrape_melon_song(GaonSong *curr_song, std::string html)
{
    ID melon_ids;
    
    // Setup parser
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    const char* html_buffer = html.c_str();
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    // Getting song id
    myhtml_collection_t *title_nodes = myhtml_get_nodes_by_attribute_value_whitespace_separated(tree, NULL, node, true, "class", 5, "title", 5, NULL);
    myhtml_tree_node_t *title_node = title_nodes->list[0];
    myhtml_tree_node_t *a_node = myhtml_node_next(title_node);  
    std::map<std::string, std::string> node_attrs = get_node_attrs(a_node);
    std::string href_string = node_attrs["href"];
    melon_ids.song_id = extract_ids_from_js(href_string, ",")[1];

    // Getting artist id
    myhtml_collection_t *a_nodes = myhtml_get_nodes_by_attribute_value(tree, NULL, node, true, "class", 5, "artist_name", 11, NULL);
    node_attrs = get_node_attrs(a_nodes->list[0]);
    href_string = node_attrs["href"];
    melon_ids.artist_id = extract_ids_from_js(href_string, "")[0];

    // Getting album id
    myhtml_collection_t *button_nodes = myhtml_get_nodes_by_attribute_key(tree, NULL, node, "data-album-no", 13, NULL);
    node_attrs = get_node_attrs(button_nodes->list[0]);
    melon_ids.album_id = std::stol(node_attrs["data-album-no"]);
  
    // Finally add to the song's final output map
    curr_song->site_ids.insert(std::pair<SITE, ID>(MELON, melon_ids));
    
    myhtml_tree_destroy(tree); 
}

void GaonParser::scrape_bugs_song(GaonSong *curr_song, std::string html)
{
    ID bugs_ids;

    // Setup parser
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    const char* html_buffer = html.c_str();
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    // Getting parent table
    myhtml_tree_node_t *table_node = myhtml_get_nodes_by_attribute_value_whitespace_separated(tree, NULL, node, true, "class", 5, "trackList", 9, NULL)->list[0];

    // Get inner tbody
    myhtml_tree_node_t *tbody_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, table_node, MyHTML_TAG_TBODY, NULL)->list[0];
    // Get inner tr nodes
    myhtml_collection_t *tr_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tbody_node, MyHTML_TAG_TR, NULL);

    // Iterate through each node until there's an mvid != 0. Then we have found our desired song
    for (int i = 0; i < tr_nodes->length; ++i) {
	std::map<std::string, std::string> attrs = get_node_attrs(tr_nodes->list[i]);
	if (std::stol(attrs["mvid"]) != 0) {
	    bugs_ids.song_id = std::stol(attrs["trackid"]);
	    bugs_ids.artist_id = std::stol(attrs["artistid"]);
	    bugs_ids.album_id = std::stol(attrs["albumid"]);
	    break;
	}
    }
    
    curr_song->site_ids.insert(std::pair<SITE, ID>(BUGS, bugs_ids));
 
    myhtml_tree_destroy(tree); 
}

void GaonParser::scrape_genie_song(GaonSong *curr_song, std::string html)
{
    ID genie_ids;

    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    const char* html_buffer = html.c_str();
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    // Getting album id
    myhtml_tree_node_t *album_node = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, node, true, "onclick", 7, "fnPlayAlbum", 11, NULL)->list[0];
    std::map<std::string, std::string> attrs = get_node_attrs(album_node);
    std::string onclick_attr = attrs["onclick"];
    genie_ids.album_id = extract_ids_from_js(onclick_attr, ",")[0];
    
    // Getting artist id
    myhtml_tree_node_t *artist_node = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, node, true, "onclick", 7, "fnViewArtist", 11, NULL)->list[0];
    attrs = get_node_attrs(artist_node);
    onclick_attr = attrs["onclick"];
    genie_ids.artist_id = extract_ids_from_js(onclick_attr, ",")[0];

    // Getting song id
    myhtml_tree_node_t *song_node = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, node, true, "onclick", 7, "fnPlaySong('", 11, NULL)->list[0];
    attrs = get_node_attrs(song_node);
    onclick_attr = attrs["onclick"];
    genie_ids.song_id = extract_ids_from_js(onclick_attr, ",")[0];

    // Finally add to the final output
    curr_song->site_ids.insert(std::pair<SITE, ID>(GENIE, genie_ids));

    myhtml_tree_destroy(tree); 
}

std::map<SITE, long> GaonParser::get_all_song_ids(myhtml_collection_t *li_nodes)
{
    myhtml_tree_node_t *melon_node = li_nodes->list[0];
    myhtml_tree_node_t *bugs_node = li_nodes->list[1];
    myhtml_tree_node_t *genie_node = li_nodes->list[2];

    myhtml_tree_node_t *nodes[] = {melon_node, bugs_node, genie_node};
    std::map<SITE, long> all_song_ids;

    // Enumerate over the SITE enums to Genie
    for (int i = MELON; i <= GENIE; ++i) {
	std::map<std::string, std::string> node_attrs = get_node_attrs(nodes[i]);
	std::string href_string = node_attrs["href"];
	
	SITE site = (SITE)i;
	all_song_ids[site] = extract_ids_from_js(href_string, ",")[1];
    }

    return all_song_ids;
}

void GaonParser::extract_all_ids(GaonSong *curr_song, myhtml_collection_t *li_nodes)
{
    std::map<SITE, long> song_ids = get_all_song_ids(li_nodes);
    for (auto const& [site, song_id] : song_ids) {
	switch(site) {
	    case MELON:
		{
		    std::string url = generate_song_url(song_id, site);
		    scrape_melon_song(curr_song, request_html(url));
		    break;
		}
	    case BUGS:
		{
		    std::string url = generate_song_url(song_id, site);
		    scrape_bugs_song(curr_song, request_html(url));
		    break;
		}
	    case GENIE:
		{
		    std::string url = generate_song_url(song_id, site);
		    scrape_genie_song(curr_song, request_html(url));
		    break;
		}
	    default:
		// TODO: ERROR
		break;
	}
    }
}

void GaonParser::load_info(GaonInfo *info)
{
    generate_url(info);
}

void GaonParser::extract_dates(SiteInfo *info, const char* html_buffer)
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

GaonSong* GaonParser::scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) 
{
    GaonSong *song_info;
    myhtml_collection_t* td_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tr_node, MyHTML_TAG_TD, NULL);
    for (int i = 0; i < td_nodes->length; ++i) {
	switch(i) {
	    case 0: 
		{    
		    //Get title and id info
		    myhtml_tree_node_t *span_node = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_SPAN, NULL)->list[0];
		    myhtml_tree_node_t *inner_text = myhtml_node_child(span_node);
		    song_info->rank = std::stoi(myhtml_node_text(inner_text, NULL));
		    break;
		}
	    case 3: 
		{
		    myhtml_collection_t *p_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_SPAN, NULL);
		    std::map<std::string, std::string> title_attrs = get_node_attrs(p_nodes->list[0]);
		    std::map<std::string, std::string> artist_attrs = get_node_attrs(p_nodes->list[1]);	
		    std::string art_alb = artist_attrs["title"];

		    size_t delim_pos = art_alb.find("|");
		    song_info->artist = art_alb.substr(0, delim_pos);
		    song_info->album = art_alb.substr(delim_pos, art_alb.length() - delim_pos - 1);
		    song_info->title = title_attrs["title"];
		    break;
		}
	    case 5:
		{
		    myhtml_collection_t *p_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_P, NULL);
		    myhtml_tree_node_t *text_node = myhtml_node_child(p_nodes->list[0]);
		    song_info->gaon_index = std::stol(myhtml_node_text(text_node, NULL));
		    break;
		}
	    case 7:
		{
		    myhtml_collection_t *li_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, td_nodes->list[i], MyHTML_TAG_LI, NULL);
		    extract_all_ids(song_info, li_nodes);
		    break;
		}
	}
    }
    return song_info;
}

std::map<int, Song*> GaonParser::parse(const char* html_buffer)
{
    // Sets up html parser and gets tree 
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_parse(tree, MyENCODING_UTF_8, html_buffer, strlen(html_buffer));
    
    myhtml_tree_node_t *node = myhtml_tree_get_document(tree);
    
    std::function<Song*(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> scrape_function = [=](myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) {
	return this->scrape_tr_nodes(tree, tr_node);
    };
    std::map<int, Song*> week_data = extract_data(tree, myhtml_node_child(node), scrape_function);
    
    myhtml_tree_destroy(tree); 
    return week_data;
}
