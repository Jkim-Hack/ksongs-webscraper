parser:
	g++ -std=c++2a -g main.cpp constants.hpp parser.hpp parser.cpp gaon_parser.cpp melon_parser.cpp bugs_parser.cpp genie_parser.cpp output_writer.hpp output_writer.cpp thread_manager.hpp thread_manager.cpp libmyhtml_static.a -lcurl

parser_o:
	g++ -std=c++2a -O main.cpp constants.hpp parser.hpp parser.cpp gaon_parser.cpp melon_parser.cpp bugs_parser.cpp genie_parser.cpp output_writer.hpp output_writer.cpp thread_manager.hpp thread_manager.cpp libmyhtml_static.a -lcurl

outputs.zip: outputs/*.csv outputs/*.xlsx
	# zip the outputs
	zip $@ $^

htmls.zip: html_downloaded/*
	# zip the outputs
	zip $@ $^
