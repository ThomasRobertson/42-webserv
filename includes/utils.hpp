#pragma once

#include <string>
#include <vector>
#include <algorithm>

#define nullptr NULL

class ContainerToStringArray
{
	public:
	ContainerToStringArray(std::vector<std::string> vec_str) : _vec_str(vec_str)
	{
	   	_vec_char = convert_to_vec_char_ptr(_vec_str);
	}
	ContainerToStringArray(std::map<std::string, std::string> map_str)
	{
    	_vec_char.reserve(map_str.size() + 1);

		for (std::vector<std::string>::iterator it = map_str.begin(); it != map_str.end(); it++)
		{
			for (std::string::iterator it_str = it->begin(); it_str != it->end(); it_str++)
				_vec_char.push_back(*it_str);
			_vec_char.push_back(nullptr);
		}
    	_vec_char.push_back(nullptr);
	}
	char* const* getArray() const {return _vec_char.data();}

	private:
	char **_array;
	std::vector<std::string> _vec_str;
	std::vector<const char*> _vec_char;

	static std::vector<const char*> convert_to_vec_char_ptr(std::vector<std::string> vec_str)
	{
		std::vector<const char*> vec_char;

		vec_char.reserve(vec_str.size() + 1);
		for (std::vector<std::string>::iterator it = vec_str.begin(); it != vec_str.end(); it++)
		{
			vec_char.push_back(it->c_str());
		}
    	vec_char.push_back(nullptr);
		return vec_char;
	}
};
