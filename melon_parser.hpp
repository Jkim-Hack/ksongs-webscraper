#include "parser.hpp"
#include <string>

typedef struct MelonInfo : SiteInfo {
    std::string gn_dp_id;
} MelonInfo;

class MelonParser : public Parser 
{ 
    private:
	void generate_url(MelonInfo *info);
    	Song scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node);
	void get_like_count(std::map<int, Song>* week_data);
	std::string generate_like_count_url(std::vector<long> song_ids);
    public:
	void load_info(MelonInfo *info);
	std::map<int, Song> parse(const char* html_buffer);
};
