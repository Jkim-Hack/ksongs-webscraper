#include <vector>
#include <string>
#include <curl/curl.h>
#include <map>

typedef struct SiteInfo {
    std::string start_date;
    std::string end_date;
    std::string url;
} SiteInfo;

typedef struct Song {
    int rank;
    long song_id;
    long number_of_likes;
    std::string title;
    std::string artist;
    std::string album;
} Song;

class Parser {
    protected:
	CURL *curl;
	std::vector<SiteInfo> site_infos;
	//size_t write_and_parse(void *ptr, size_t size, size_t nmemb, std::string *data);
	const char* generate_url(SiteInfo info);
	virtual std::map<int, Song> parse(const char* html_buffer) = 0;
    public:
	std::map<long, std::map<int, Song>> extracted_data;
	Parser();
	~Parser();
	void load_info(SiteInfo site_info);
	void prepare_handle(int index);
	void perform_all_handles();
};
