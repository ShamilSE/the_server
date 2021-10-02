#include "../../includes/utils.hpp"

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