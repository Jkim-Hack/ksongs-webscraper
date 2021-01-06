#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include "constants.hpp"
#include <iostream>
#include <regex>
#include <memory>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <myhtml/api.h>
#include <map>
#include <array>
#include <sstream>
#include <exception>

class HTMLScrapeFailureException : public std::exception {

    std::string site;
    std::string section;
    
    public: 
	HTMLScrapeFailureException(std::string site, std::string section) {
	    this->site = site;
	    this->section = section;
	}
	const char* what() const throw() {
	    std::string message("The site ");
	    message += site + " failed to scrape section: " + section;
	    return message.c_str();
	}
};

struct SiteInfo {
    std::string start_date;
    std::string end_date;
    std::string url;
    SITE site_type;
    long key_hash;

    // Only care about the start date
    const bool operator==(const SiteInfo &o) {
        return start_date.compare(o.start_date) == 0;
    }

    const bool operator<(const SiteInfo &o) {
        return std::stol(start_date) < std::stol(o.start_date);
    }

    virtual ~SiteInfo() = default;
};

struct Song {
    int rank;
    std::string title;
    std::vector<std::string> artists;
    std::string album;
    virtual ~Song() = default;
};

class Parser {
    protected:
	CURL *curl;
	myhtml_t* myhtml;
	virtual void generate_url(std::shared_ptr<SiteInfo> info) = 0;
	std::string extract_id(std::string text);
	static bool find_case_insensitive(std::string str1, std::string str2);
	static std::vector<long> extract_ids_from_js(std::string attr_string);
	static std::map<std::string, std::string> get_node_attrs(myhtml_tree_node_t *node);
    	static size_t write(void *ptr, size_t size, size_t nmemb, std::string *data);
	virtual std::shared_ptr<Song> scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) = 0;
    public:
	Parser();
	~Parser();
	std::map<SiteInfo, std::map<int, std::shared_ptr<Song>>> extracted_data;
	virtual void load_info(std::shared_ptr<SiteInfo> site_info, size_t max_dist_size) = 0;
	std::string request_html(std::string url); 
	std::map<int, std::shared_ptr<Song>> extract_data(myhtml_tree_t* tree, myhtml_tree_node_t *node, int starting, std::function<std::shared_ptr<Song>(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> &scrape_function);
	virtual std::map<int, std::shared_ptr<Song>> parse(const char* html_buffer) = 0;
};

// MELON

struct MelonInfo : SiteInfo {
    std::string gn_dp_id;
    virtual ~MelonInfo() = default;
};

struct ID {
    long song_id;
    long album_id;
    std::vector<long> artist_ids;
};

struct MelonSong : Song {
    long song_id;
    long artist_id;
    long album_id;
    long number_of_likes;
    virtual ~MelonSong() = default;
};

class MelonParser : public Parser 
{ 
    private:
	void generate_url(std::shared_ptr<SiteInfo> info) override;
	std::shared_ptr<Song> scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) override;
	void get_like_count(std::map<int, std::shared_ptr<Song>>* week_data);
	std::string generate_like_count_url(std::vector<long> song_ids);
    public:
	void load_info(std::shared_ptr<SiteInfo> info, size_t max_dist_size) override;
	std::map<int, std::shared_ptr<Song>> parse(const char* html_buffer) override;
	static void scrape_album(ID *ids, myhtml_tree_t* tree, myhtml_tree_node_t* node, std::string target_title);
};

// GAON

enum GAON_TYPE {
    DIGITAL = 0x01,
    DOWNLOAD = 0x02,
    STREAMING = 0x03
};

struct GaonInfo : SiteInfo {
    int week;
    int year;
    enum GAON_TYPE type;
    virtual ~GaonInfo() = default;
};

struct GaonSong : Song {
    long gaon_index;
    std::map<SITE, ID> site_ids;
    virtual ~GaonSong() = default;
};

class GaonParser : public Parser
{
    private:
	void generate_url(std::shared_ptr<SiteInfo> info) override;
	std::string generate_song_url(long song_id, enum SITE site);
	std::map<SITE, long> get_all_song_ids(myhtml_collection_t *li_nodes);
	void extract_all_ids(std::shared_ptr<GaonSong> curr_song, myhtml_collection_t *li_nodes);
    public:
	void scrape_melon_song(std::shared_ptr<GaonSong> curr_song, std::string html);
	void scrape_bugs_song(std::shared_ptr<GaonSong> curr_song, std::string html);
	void scrape_genie_song(std::shared_ptr<GaonSong> curr_song, std::string html);
	void load_info(std::shared_ptr<SiteInfo> info, size_t max_dist_size) override;
	void extract_dates(std::shared_ptr<SiteInfo> info, const char* html_buffer);
	std::shared_ptr<Song> scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) override;
	std::map<int, std::shared_ptr<Song>> parse(const char* html_buffer) override;
};

struct BugsSong : Song {
    long song_id;
    long album_id;
    std::vector<long> artist_ids;
};

class BugsParser : public Parser 
{
    private:
	void generate_url(std::shared_ptr<SiteInfo> info) override;
    public:
	void load_info(std::shared_ptr<SiteInfo> info, size_t max_dist_size) override;
	std::shared_ptr<Song> scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) override;
	std::map<int, std::shared_ptr<Song>> parse(const char* html_buffer) override;
	static void scrape_album(ID *ids, myhtml_tree_t* tree, myhtml_tree_node_t* node, std::string target_title);
};

struct GenieSong : Song {
    long song_id;
    long album_id;
    long artist_id;
};

class GenieParser : public Parser 
{
    private:
	void generate_url(std::shared_ptr<SiteInfo> info) override;
    public:
	void load_info(std::shared_ptr<SiteInfo> info, size_t max_dist_size) override;
	std::shared_ptr<Song> scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node) override;
	std::map<int, std::shared_ptr<Song>> parse(const char* html_buffer) override;
	static void scrape_album(ID *ids, myhtml_tree_t* tree, myhtml_tree_node_t* node, std::string target_title);
};
#endif
