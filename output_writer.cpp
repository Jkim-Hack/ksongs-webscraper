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

void OutputWriter::execute_output(std::map<std::shared_ptr<SiteInfo>, std::map<int, std::shared_ptr<Song>>>* data)
{
    std::ofstream output(this->filepath);
    
    // Data headers
    switch (this->site_to_use) {
	case MELON:
	    {
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album" << "," << "# Likes" << "," << "Artist ID" << "," << "Album ID";
		// Data inputs
		for (auto const& [site_info, song_info] : *data) {
		    for (auto const& [rank, song] : song_info) {
			std::stringstream buffer;
			std::shared_ptr<MelonSong> melon_song = std::dynamic_pointer_cast<MelonSong>(song);
			/*
			std::istringstream end_date_stream(site_info->start_date);

			struct std::tm tm;
			end_date_stream >> std::get_time(&tm, "%Y%m%d");
			std::time_t end_time_t = mktime(&tm);
			auto end_sd = std::chrono::system_clock::from_time_t(end_time_t) + std::chrono::days(6);
			std::time_t end_date_c = std::chrono::system_clock::to_time_t(end_sd);
			std::tm curr_end_date = *std::localtime(&end_date_c);
			buffer << std::put_time(&curr_end_date, "%Y%m%d");
			*/
			std::string  start_date_normalize = normalize_date(site_info->start_date);
			std::string end_date_normalize = normalize_date(site_info->end_date);
			output << start_date_normalize << "," <<  end_date_normalize << "," << melon_song->rank << ",\"" << melon_song->title << "\",\""; 
			for (auto const& artist : melon_song->artists) {
			    output << artist << " | "; 
			}
			output << "\",\"" << melon_song->album << "\"," << melon_song->number_of_likes << "," << melon_song->artist_id << "," << melon_song->album_id;
			output << "\n";
		    }
		}
		break;
	    }
	case GAON_DIGITAL:
	    {
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album" << "," << "Gaon Index" << "," << "Melon Song ID" << "," << "Melon Artist ID" << "," << "Melon Album ID" << "," << "Bugs Song ID" << "," << "Bugs Artist ID" << "," << "Bugs Album ID" << "," << "Genie Song ID" << "," << "Genie Artist ID" << "," << "Genie Album ID";
		output << "\n";
		// Data inputs
		for (auto const& [site_info, song_info] : *data) {
		    for (auto const& [rank, song] : song_info) {
			std::shared_ptr<GaonSong> gaon_song = std::dynamic_pointer_cast<GaonSong>(song);
			
			std::string start_date_normalize = normalize_date(site_info->start_date);
			std::string end_date_normalize = normalize_date(site_info->end_date);
			
			output << start_date_normalize << "," <<  end_date_normalize << "," << gaon_song->rank << ",\"" << gaon_song->title << "\",\""; 
			for (size_t j = 0; j < gaon_song->artists.size(); ++j) {
			    if (j == gaon_song->artists.size() - 1) {
				output << gaon_song->artists[j]; 
			    } else {
				output << gaon_song->artists[j] << " | "; 
			    }
			}
			output << "\",\"" << gaon_song->album << "\"";
			output << "," << gaon_song->gaon_index;
			for (auto const& [site, id] : gaon_song->site_ids) {
			    output << "," << id.song_id << ",";
			    for (size_t i = 0; i < id.artist_ids.size(); ++i) {
				if (i + 1 == id.artist_ids.size()) {
				    output << id.artist_ids[i] << ","; 
				} else {
				    output << id.artist_ids[i] << " | "; 
				}
			    }
			    output << id.album_id;
			}
			output << "\n";
		    }
		}
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
