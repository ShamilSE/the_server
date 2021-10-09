#include "../includes/utils.hpp"

void	ft_memcpy(const void *dst, const void *src, size_t n)
{
	unsigned long* tmp1;
	unsigned long * tmp2;
	unsigned char *c_dst;
	unsigned char *c_src;

	tmp1 = (unsigned long *)dst;
	tmp2 = (unsigned long *)src;
	for (ssize_t i = 0, m = n / sizeof(long); i < m; ++i )
		*tmp1++ = *tmp2++;

	c_dst = reinterpret_cast<unsigned char *>(tmp1);
	c_src = reinterpret_cast<unsigned char *>(tmp2);
	for (ssize_t i = 0, m = n % sizeof(long ); i < m; ++i)
		*c_dst++ = *c_src++;
}

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

size_t	ft_strlen(const char *str)
{
	int count;

	count = 0;
	while (str[count] != 0)
		count++;
	return (count);
}

char	*ft_strdup(const char *s1)
{
	char	*dst;
	int		i;

	i = -1;
	if (!(dst = (char*)malloc((ft_strlen(s1) + 1))))
		return (0);
	while (s1[++i] != 0)
		dst[i] = s1[i];
	dst[i] = 0;
	return (dst);
}

char	*ft_strjoin(const char *s1, const char *s2)
{
	char	*dst;
	int		count;

	if (!s1 || !s2 || !(dst = (char *)malloc((ft_strlen(s1) + ft_strlen(s2) + 1))))
		return (0);
	count = 0;
	while (*s1 != 0)
		dst[count++] = *s1++;
	while (*s2 != 0)
		dst[count++] = *s2++;
	dst[count] = 0;
	return (dst);
}

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
