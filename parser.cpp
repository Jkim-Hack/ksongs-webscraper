#include "parser.hpp"

std::ofstream err_output;

Parser::Parser() 
{
    if (!err_output.is_open()) {
	err_output.open("error.txt");
    }
    curl = curl_easy_init();
    this->myhtml = myhtml_create();
    myhtml_init(this->myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
}

Parser::~Parser() 
{
    curl_easy_cleanup(this->curl);
    myhtml_destroy(this->myhtml);
}

std::string Parser::remove_junk_spaces(std::string text) noexcept
{
    std::smatch match;
    std::regex word_regex("[^\\s]");
    std::string result_word = text;
    
    size_t first_pos = 0;
    if (std::regex_search(text, match, word_regex)) {
	first_pos = match.position();
    }
    result_word = result_word.substr(first_pos, result_word.length() - first_pos);
    result_word.erase(std::remove(result_word.begin(), result_word.end(), '\n'), result_word.end());
    return result_word;
}

bool check_char_equality(char ch1, char ch2) noexcept
{
    return std::toupper(ch1) == std::toupper(ch2);
}

bool Parser::find_case_insensitive(std::string str1, std::string str2) noexcept
{
    std::string str1_inner_parenth, str2_inner_parenth;

    str1.erase(std::remove(str1.begin(), str1.end(), ' '), str1.end());
    str2.erase(std::remove(str2.begin(), str2.end(), ' '), str2.end());
   
    size_t str1_left_parenth = str1.find_first_of("(");
    size_t str2_left_parenth = str2.find_first_of("(");

    if (str1_left_parenth != std::string::npos) {
	str1_inner_parenth = str1.substr(str1_left_parenth + 1, str1.length() - str1_left_parenth - 2);	
	str1 = str1.substr(0, str1_left_parenth);
	auto iterator_parenth_one_two = std::search(str2.begin(), str2.end(), str1_inner_parenth.begin(), str1_inner_parenth.end(), check_char_equality);
	if (iterator_parenth_one_two != str2.end()) return true;    
    }

    if (str2_left_parenth != std::string::npos) {
	str2_inner_parenth = str2.substr(str2_left_parenth + 1, str2.length() - str2_left_parenth - 2);
	str2 = str2.substr(0, str2_left_parenth);
	auto iterator_parenth_two_one = std::search(str1.begin(), str1.end(), str2_inner_parenth.begin(), str2_inner_parenth.end(), check_char_equality);
	if (iterator_parenth_two_one != str1.end()) return true;
    }

    std::smatch match;
    std::regex word_regex ("[^.,’`~;'!?@#$%^&*()′]");
    std::string t_str1, t_str2; // "tailored" strings

    while (std::regex_search(str1, match, word_regex)) {
	t_str1 += match.str();
	str1 = match.suffix().str();
    }

    while (std::regex_search(str2, match, word_regex)) {
	t_str2 += match.str();
	str2 = match.suffix().str();
    }
    
    auto iterator_one_two = std::search(t_str1.begin(), t_str1.end(), t_str2.begin(), t_str2.end(), check_char_equality);
    if (iterator_one_two != t_str1.end()) return true;
    
    auto iterator_two_one = std::search(t_str2.begin(), t_str2.end(), t_str1.begin(), t_str1.end(), check_char_equality);
    if (iterator_two_one != t_str2.end()) return true;
     
    return false;
}

std::vector<long> Parser::extract_ids_from_js(std::string attr_string) noexcept
{
    std::vector<long> ids;
    std::smatch match;
    std::regex digit_regex ("(\\d+)");

    while (std::regex_search(attr_string, match, digit_regex)) {
	ids.push_back(std::stol(match.str()));
	attr_string = match.suffix().str();
    }

    return ids;
}

std::map<std::string, std::string> Parser::get_node_attrs(myhtml_tree_node_t *node) noexcept
{
    std::map<std::string, std::string> attributes;
    myhtml_tree_attr_t *attr = myhtml_node_attribute_first(node);
    
    while (attr) {
        const char *name = myhtml_attribute_key(attr, NULL);
	if(name) {
            const char *value = myhtml_attribute_value(attr, NULL);
	    attributes.insert(std::pair<std::string, std::string>(name, value));
	}
        attr = myhtml_attribute_next(attr);
    }
    return attributes;
}

size_t Parser::write(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

// Prepare libcurl for get request
std::string Parser::request_html(std::string url) 
{
    struct curl_slist *chunk = NULL;
    std::string response_string;
    if (curl) {
	chunk = curl_slist_append(chunk, "Cookie: PCID=16080592268936819734880; PC_PCID=16080592268936819734880; POC=MP10");
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20); // 10 sec timeout
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write);    
	std::cout << url.c_str() << std::endl;
	curl_easy_perform(curl);
    }
    return response_string;
}

std::map<int, std::shared_ptr<Song>> Parser::extract_data(myhtml_tree_t *tree, myhtml_tree_node_t *node, int starting, std::function<std::shared_ptr<Song>(myhtml_tree_t* tree, myhtml_tree_node_t *tr_node)> &scrape_function)
{
    std::map<int, std::shared_ptr<Song>> week_data;
    myhtml_collection_t* tbody_nodes = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_TBODY, NULL);
    if (tbody_nodes->length > 0) {
	myhtml_collection_t* tr_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, tbody_nodes->list[0], MyHTML_TAG_TR, NULL);
	std::cout << tr_nodes->length << std::endl;
	for (int i = starting; i < tr_nodes->length; ++i) {
	    std::cout << i << std::endl;
	    std::shared_ptr<Song> song_info = scrape_function(tree, tr_nodes->list[i]);
	    week_data.insert(std::pair<int, std::shared_ptr<Song>>(song_info->rank, song_info));
	}
	std::cout << "Extract data done\n";
    } else {
	std::cout << "INVALID HTML" << std::endl;
    }
    return week_data;
}
