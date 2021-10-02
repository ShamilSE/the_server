#include "../../includes/utils.hpp"

std::string		ft_strtrim(const std::string &str, const std::string &set)
{
	size_t		begin = 0;
	size_t		end = str.length() - 1;

	while (str[begin] && set.find(str[begin]) != std::string::npos)
		++begin;
	while (end && set.find(str[end]) != std::string::npos)
		--end;
	return str.substr(begin, end - begin + 1);
}
