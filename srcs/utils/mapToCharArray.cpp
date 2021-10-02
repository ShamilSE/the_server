#include "../../includes/utils.hpp"

char** mapToCharArray(std::map<std::string, std::string> map, std::string delimeter) {
	char** array = new char*[map.size() + 1];

	std::map<std::string, std::string>::iterator it = map.begin();
	// array[0] = new char[500];
	// strcpy(array[0], "/Users/mismene/ft_webserver/cgi/cgi_tester");
	// array[0][43] = 0;
	for (size_t index = 1; index < map.size(); index++, it++) {
		array[index] = new char[(it->first + delimeter + it->second).size()];
		strcpy(array[index], (it->first + delimeter + it->second).c_str());
	}
	array[map.size()] = NULL;
	return array;
}