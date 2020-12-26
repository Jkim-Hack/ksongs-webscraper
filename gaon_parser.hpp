#include "parser.hpp"
#include <string>
#include <myhtml/api.h>

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

typedef struct ID {
    long song_id;
    long artist_id;
    long album_id;
} ID;

typedef struct GaonSong : Song {
    long gaon_index;
    std::map<SITE, ID> site_ids;
} GaonSong;

class GaonParser : Parser 
{
    private:
	void generate_url(GaonInfo *info);
	std::string generate_song_url(long song_id, enum SITE site);
	void scrape_melon_song(GaonSong *curr_song, std::string html);
	void scrape_bugs_song(GaonSong *curr_song, std::string html);
	void scrape_genie_song(GaonSong *curr_song, std::string html);
	std::map<SITE, long> get_all_song_ids(myhtml_collection_t *li_nodes);
	void extract_all_ids(GaonSong *curr_song, myhtml_collection_t *li_nodes);
    public:
	void load_info(GaonInfo *info);
	void extract_dates(SiteInfo *info, const char* html_buffer);
	GaonSong* scrape_tr_nodes(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node);
	std::map<int, Song*> parse(const char* html_buffer);
};
