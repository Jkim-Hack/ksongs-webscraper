parser:
	g++ -std=c++2a main.cpp constants.hpp parser.hpp parser.cpp gaon_parser.cpp melon_parser.cpp bugs_parser.cpp genie_parser.cpp output_writer.hpp output_writer.cpp thread_manager.hpp thread_manager.cpp libmyhtml_static.a -lcurl

parser_o:
	g++ -std=c++2a -O main.cpp constants.hpp parser.hpp parser.cpp gaon_parser.cpp melon_parser.cpp bugs_parser.cpp genie_parser.cpp output_writer.hpp output_writer.cpp thread_manager.hpp thread_manager.cpp libmyhtml_static.a -lcurl

scrapers:
	g++ -std=c++2a p_scrapers.cpp constants.hpp parser.hpp parser.cpp gaon_parser.cpp melon_parser.cpp bugs_parser.cpp genie_parser.cpp output_writer.hpp output_writer.cpp thread_manager.hpp thread_manager.cpp libmyhtml_static.a -lcurl
