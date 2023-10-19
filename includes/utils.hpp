#pragma ounce

#include <string>
#include <vector>

class VectorToStringArray
{
	public:
	VectorToStringArray(std::vector<std::string> vec_str);
	char* const* getArray() const {return _vec_char.data();}

	private:
	char **_array;
	std::vector<std::string> _vec_str;
	std::vector<char*> _vec_char;
};
