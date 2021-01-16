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
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album" << "," << "# of Likes" << "," << "Song ID" << "," << "Artist ID" << "," << "Album ID";
		output << "\n";
		// Data inputs
		for (auto const& [site_info, song_info] : *data) {
		    for (auto const& [rank, song] : song_info) {
			std::stringstream buffer;
			std::shared_ptr<MelonSong> melon_song = std::dynamic_pointer_cast<MelonSong>(song);
			std::string  start_date_normalize = normalize_date(site_info->start_date);
			std::string end_date_normalize = normalize_date(site_info->end_date);
			output << start_date_normalize << "," <<  end_date_normalize << "," << melon_song->rank << ",\"" << melon_song->title << "\",\""; 
			for (size_t j = 0; j < melon_song->artists.size(); ++j) {
			    if (j == melon_song->artists.size() - 1) {
				output << melon_song->artists[j]; 
			    } else {
				output << melon_song->artists[j] << " | "; 
			    }
			}
			output << "\",\"" << melon_song->album << "\"," << melon_song->number_of_likes << "," << melon_song->song_id << ",";
			for (size_t j = 0; j < melon_song->artist_ids.size(); ++j) {
			    if (j == melon_song->artist_ids.size() - 1) {
				output << melon_song->artist_ids[j]; 
			    } else {
				output << melon_song->artist_ids[j] << " | "; 
			    }
			}
			output << "," << melon_song->album_id;	
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
		std::map<std::shared_ptr<SiteInfo>, std::map<int, std::shared_ptr<Song>>>::iterator iter = data->begin();
		for (; iter != data->end(); ++iter) {
		    std::map<int, std::shared_ptr<Song>>::iterator week_iterator = iter->second.begin();
		    for (; week_iterator != iter->second.end(); ++week_iterator) {
			std::shared_ptr<SiteInfo> site_info = iter->first;
			std::shared_ptr<Song> song = week_iterator->second;
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
			    if (id.artist_ids.size() < 1) {
				output << 0;
			    } else {
				for (size_t i = 0; i < id.artist_ids.size(); ++i) {
				    if (i + 1 == id.artist_ids.size()) {
					output << id.artist_ids[i]; 
				    } else {
					output << id.artist_ids[i] << " | "; 
				    }
				}
			    }
			    output << "," << id.album_id;
			}
			output << "\n";
		    }
		}
		break;
	    }
	case GAON_DOWNLOAD:
	    {
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album"  << "," << "Melon Song ID" << "," << "Melon Artist ID" << "," << "Melon Album ID" << "," << "Bugs Song ID" << "," << "Bugs Artist ID" << "," << "Bugs Album ID" << "," << "Genie Song ID" << "," << "Genie Artist ID" << "," << "Genie Album ID";
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
			for (auto const& [site, id] : gaon_song->site_ids) {
			    output << "," << id.song_id << ",";
			    if (id.artist_ids.size() < 1) {
				output << 0;
			    } else {
				for (size_t i = 0; i < id.artist_ids.size(); ++i) {
				    if (i + 1 == id.artist_ids.size()) {
					output << id.artist_ids[i]; 
				    } else {
					output << id.artist_ids[i] << " | "; 
				    }
				}
			    }
			    output << "," << id.album_id;
			}
			output << "\n";
		    }
		}
		break;
	    }
	case GAON_STREAMING:
	    {
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album" << "," << "Melon Song ID" << "," << "Melon Artist ID" << "," << "Melon Album ID" << "," << "Bugs Song ID" << "," << "Bugs Artist ID" << "," << "Bugs Album ID" << "," << "Genie Song ID" << "," << "Genie Artist ID" << "," << "Genie Album ID";
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
			for (auto const& [site, id] : gaon_song->site_ids) {
			    output << "," << id.song_id << ",";
			    if (id.artist_ids.size() < 1) {
				output << 0;
			    } else {
				for (size_t i = 0; i < id.artist_ids.size(); ++i) {
				    if (i + 1 == id.artist_ids.size()) {
					output << id.artist_ids[i]; 
				    } else {
					output << id.artist_ids[i] << " | "; 
				    }
				}
			    }
			    output << "," << id.album_id;
			}
			output << "\n";
		    }
		}
		break;
	    }
	case BUGS:
	    {
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album" << "," << "Song ID" << "," << "Artist ID" << "," << "Album ID";
		output << "\n";
		// Data inputs
		for (auto const& [site_info, song_info] : *data) {
		    for (auto const& [rank, song] : song_info) {
			std::stringstream buffer;
			std::shared_ptr<BugsSong> bugs_song = std::dynamic_pointer_cast<BugsSong>(song);
			std::string start_date_normalize = normalize_date(site_info->start_date);
			std::string end_date_normalize = normalize_date(site_info->end_date);
			output << start_date_normalize << "," <<  end_date_normalize << "," << bugs_song->rank << ",\"" << bugs_song->title << "\",\""; 
			for (size_t j = 0; j < bugs_song->artists.size(); ++j) {
			    if (j == bugs_song->artists.size() - 1) {
				output << bugs_song->artists[j]; 
			    } else {
				output << bugs_song->artists[j] << " | "; 
			    }
			}

			output << "\",\"" << bugs_song->album << "\"," << bugs_song->song_id << ","; 
			
			for (size_t j = 0; j < bugs_song->artist_ids.size(); ++j) {
			    if (j == bugs_song->artist_ids.size() - 1) {
				output << bugs_song->artist_ids[j]; 
			    } else {
				output << bugs_song->artist_ids[j] << " | "; 
			    }
			}
			output << "," << bugs_song->album_id;
			output << "\n";
		    }
		}
		break;
	    }
	case GENIE:
	    {
		output << "Start Date" << "," << "End Date" << "," << "Rank" << "," << "Title" << "," << "Artist" << "," << "Album" << "," << "Song ID" << "," << "Artist ID" << "," << "Album ID";
		output << "\n";
		// Data inputs
		for (auto const& [site_info, song_info] : *data) {
		    for (auto const& [rank, song] : song_info) {
			std::stringstream buffer;
			std::shared_ptr<GenieSong> genie_song = std::dynamic_pointer_cast<GenieSong>(song);
			std::string start_date_normalize = normalize_date(site_info->start_date);
			std::string end_date_normalize = normalize_date(site_info->end_date);
			output << start_date_normalize << "," <<  end_date_normalize << "," << genie_song->rank << ",\"" << genie_song->title << "\",\""; 
			for (size_t j = 0; j < genie_song->artists.size(); ++j) {
			    if (j == genie_song->artists.size() - 1) {
				output << genie_song->artists[j]; 
			    } else {
				output << genie_song->artists[j] << " | "; 
			    }
			}

			output << "\",\"" << genie_song->album << "\"," << genie_song->song_id << "," << genie_song->artist_id << "," << genie_song->album_id;
			output << "\n";
		    }
		}
		break;
	    }
    }
    output << "\n";


    output.close();
}
