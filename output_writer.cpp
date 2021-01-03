#include "output_writer.hpp"
#include <chrono>
#include <fstream>
#include <sstream>

void OutputWriter::set_output(std::string filepath)
{
    this->filepath = filepath;
}

void OutputWriter::set_site_to_use(SITE site_to_use)
{
    this->site_to_use = site_to_use;
}

std::string normalize_date(std::string date) 
{
    date.insert(4, "/");
    date.insert(7, "/");
    return date;
}

void OutputWriter::execute_output(std::map<std::string, std::map<int, Song*>> data)
{
    std::ofstream output(this->filepath);
    
    // Data headers
    switch (this->site_to_use) {
	case MELON:
	    {
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album" << "," << "# Likes" << "," << "Artist ID" << "," << "Album ID";
		// Data inputs
		for (auto week_data : data) {
		    for (auto song_data : week_data.second) {
			std::stringstream buffer;
			MelonSong* song = (MelonSong *)song_data.second;
			std::istringstream end_date_stream(week_data.first);

			struct std::tm tm;
			end_date_stream >> std::get_time(&tm, "%Y%m%d");
			std::time_t end_time_t = mktime(&tm);
			auto end_sd = std::chrono::system_clock::from_time_t(end_time_t) + std::chrono::days(6);
			std::time_t end_date_c = std::chrono::system_clock::to_time_t(end_sd);
			std::tm curr_end_date = *std::localtime(&end_date_c);
			buffer << std::put_time(&curr_end_date, "%Y%m%d");

			std::string  start_date_normalize = normalize_date(week_data.first);
			std::string end_date_normalize = normalize_date(buffer.str());
			
			output << start_date_normalize << "," <<  end_date_normalize << "," << song->rank << ",\"" << song->title << "\",\"" << song->artist << "\",\"" << song->album << "\"," << song->number_of_likes << "," << song->artist_id << "," << song->album_id;
			output << "\n";
		    }
		}
		break;
	    }
	case GAON_DIGITAL:
	    {
		// TODO: IMPLEMENT
		break;
	    }
	case GAON_DOWNLOAD:
	    break;
	case GAON_STREAMING:
	    break;
	case BUGS:
	    break;
	case GENIE:
	    break;
    }
    output << "\n";


    output.close();
}
