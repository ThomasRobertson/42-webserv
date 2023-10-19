#include "utils.hpp"

VectorToStringArray::VectorToStringArray(std::vector<std::string> vec_str) : _vec_str(vec_str)
{
	std::vector<char*> _vec_char;
    _vec_char.reserve(_vec_str.size() + 1);

    std::transform(begin(_vec_str), end(_vec_str), std::back_inserter(_vec_char),
                   [](std::string &s) { return const_cast<char*>(s.data()); }
                  );
    _vec_char.push_back(nullptr);
}
