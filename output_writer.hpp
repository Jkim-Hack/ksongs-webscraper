#ifndef __OUTPUT_WRITER_HPP__
#define __OUTPUT_WRITER_HPP__

#include "thread_manager.hpp"
#include <string>
#include <map>

class OutputWriter {
    private:
	SITE site_to_use;
	std::string filepath;
    public:
	OutputWriter(): site_to_use(MELON), filepath("output.csv"){};
	OutputWriter(SITE site_to_use): site_to_use(site_to_use), filepath("output.csv"){};
	OutputWriter(SITE site_to_use, std::string filepath): site_to_use(site_to_use), filepath(filepath){};
	void set_output(std::string filepath);
	void set_site_to_use(SITE site_to_use);
	static void write_html(std::string path, std::string html);
	void execute_output(std::map<std::shared_ptr<SiteInfo>, std::map<int, std::shared_ptr<Song>>>* data);
};

#endif
