parser:
	g++ -std=c++2a main.cpp constants.hpp melon_parser.hpp melon_parser.cpp parser.hpp parser.cpp output_writer.hpp output_writer.cpp libmyhtml_static.a -lcurl

parser_o:
	g++ -std=c++2a -O main.cpp constants.hpp melon_parser.hpp melon_parser.cpp parser.hpp parser.cpp output_writer.hpp output_writer.cpp libmyhtml_static.a -lcurl
