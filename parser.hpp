#include <vector>
#include <string>
#include <curl/curl.h>
#include <myhtml/api.h>
#include <map>

typedef struct SiteInfo {
    std::string start_date;
    std::string end_date;
    std::string url;
    
    // Only care about the start date
    const bool operator==(const SiteInfo &o) {
        return start_date.compare(o.start_date) == 0;
    }

    const bool operator<(const SiteInfo &o) {
        return std::stol(start_date) < std::stol(o.start_date);
    }

} SiteInfo;

typedef struct Song {
    int rank;
    long song_id;
    long number_of_likes;
    long gaon_index;
    std::string title;
    std::string artist;
    std::string album;
    std::string artist_id;
    std::string album_id;
} Song;

class Parser {
    protected:
	CURL *curl;
	virtual void generate_url(SiteInfo *info) = 0;
	std::string extract_id(std::string text);
	std::map<std::string, std::string> get_node_attrs(myhtml_tree_node_t *node);
    	static size_t write(void *ptr, size_t size, size_t nmemb, std::string *data);
	virtual Song scrape_tr_nodes(myhtml_tree_node_t* tree, myhtml_tree_node_t *tr_node) = 0;
    public:
	Parser();
	~Parser();
	std::map<SiteInfo, std::map<int, Song>> extracted_data;
	virtual void load_info(SiteInfo *site_info) = 0;
	std::string request_html(SiteInfo *info);
	std::map<int, Song> extract_data(myhtml_tree_t* tree, myhtml_tree_node_t *node, std::function<Song(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> &scrape_function);
	virtual std::map<int, Song> parse(const char* html_buffer) = 0;
};
