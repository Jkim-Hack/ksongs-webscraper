#include "parser.hpp"
#include <string>

typedef struct MelonInfo : SiteInfo {
    std::string gn_dp_id;
} MelonInfo;

class MelonParser : public Parser 
{ 
    private:
	std::string generate_url(MelonInfo info);
	std::string generate_like_count_url(std::vector<long> song_ids);
	void get_like_count(std::map<int, Song>* week_data);
	static size_t write(void *ptr, size_t size, size_t nmemb, std::string *data);	
    public:
	void prepare_handle(int index);
	void load_info(MelonInfo info);
	const char* request_html();
	std::map<int, Song> parse(const char* html_buffer);
};
