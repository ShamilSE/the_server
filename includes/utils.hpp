#pragma once

#include <iostream>
#include <vector>
#include <map>

std::string					ft_strtrim(const std::string &, const std::string &);
std::vector<std::string>	ft_split(std::string str, const std::string &del);
char** mapToCharArray(std::map<std::string, std::string> map, std::string delimeter);
char	*ft_strdup(const char *s1);
size_t	ft_strlen(const char *str);
char	*ft_strjoin(const char *s1, const char *s2);
