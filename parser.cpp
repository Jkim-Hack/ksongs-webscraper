#include "parser.hpp"
#include <string>
#include <curl/curl.h>

Parser::Parser(SiteInfo site_info) 
{
    Parser::load_info(site_info);
}
