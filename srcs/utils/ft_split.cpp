#include "../../includes/utils.hpp"

std::vector<std::string>	ft_split(std::string str, const std::string &del)
{
	std::vector<std::string>	splitStr;
	size_t						pos;

	if (!str.empty()) {
		if (del.empty()) {
			splitStr.push_back(str);
		} else {
			str = ft_strtrim(str, del);
			if (!str.empty()) {
				for ( pos = str.find(del) ; pos != std::string::npos ; pos = str.find(del) ) {
					if (str.substr(0, pos) != "")
						splitStr.push_back(str.substr(0, pos));
					str = str.substr(pos + del.length(), str.length() - pos);
				}
				splitStr.push_back(str.substr(0, pos));
			}
		}
	}
	return splitStr;
}
