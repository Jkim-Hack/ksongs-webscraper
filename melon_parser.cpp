#include "melon_parser.hpp"

const char* MelonParser::generate_url()
{ 
    MelonInfo info = this->melon_info;
    std::string start_year = info.start_date.substr(0, 4);
    int start_year_int = std::stoi(start_year.c_str());
    std::string url;
    if (start_year_int >= 2020) {
	url = "https://www.melon.com/chart/week/index.htm?classCd=GN0000&moved=Y&startDay=" + info.start_date + "&endDay=" + info.end_date;
    } else if (start_year_int < 2020 && start_year_int >= 2010) {
	std::string month = info.start_date.substr(4, 2);
	url = "https://www.melon.com/chart/search/list.htm?chartType=WE&age=2010&year=" + start_year + "&mon=" + month + "&day=" + info.start_date +"^" + info.end_date + "&classCd="+ info.gn_dn_id +"0000&startDay="+info.start_date+"&endDay="+info.end_date+"&moved=Y";
    } else {
	// TODO: Throw exception
    }
    return url.c_str();
}
