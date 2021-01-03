#pragma once

#ifndef __OUTPUT_WRITER_H__
#define __OUTPUT_WRITER_H__

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
	void execute_output(std::map<std::string, std::map<int, Song*>> data);
};

#endif
