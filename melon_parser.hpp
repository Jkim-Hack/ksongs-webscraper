#include "parser.hpp"
#include "melon.hpp"

class MelonParser : public Parser 
{
    MelonInfo melon_info;
   
    public:
	void load_info(MelonInfo info);
    private:
	const char* generate_url();
};
