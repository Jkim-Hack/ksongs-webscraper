#include "site_info.hpp"

class Parser {
    protected:
    	SiteInfo site_info;
	virtual const char* generate_url() = 0;
    public:
	Parser(SiteInfo site_info);
	virtual void parse() = 0;
	void load_info(SiteInfo site_info);
};
