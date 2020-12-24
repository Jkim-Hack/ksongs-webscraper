#include "parser.hpp"
#include <string>

enum GAON_TYPE {
    DIGITAL = 0x01,
    DOWNLOAD = 0x02,
    STREAMING = 0x03
};

typedef struct GaonInfo : SiteInfo {
    int week;
    int year;
    enum GAON_TYPE type;
} GaonInfo;

class GaonParser : Parser 
{
    private:
	void generate_url(GaonInfo *info);
	std::vector<long> extract_song_ids(std::map<int, Song> week_data);
    public:
	void load_info(GaonInfo *info);
	void extract_dates(SiteInfo *info, const char* html_buffer);
	Song scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node);
	std::map<int, Song> parse(const char* html_buffer);
};
