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
	std::string generate_url(GaonInfo info);
	static size_t write(void *ptr, size_t size, size_t nmemb, std::string *data);
    public:
	void prepare_handle(int index);
	void load_info(GaonInfo info);
	std::map<int, Song> parse(const char* html_buffer);
};
