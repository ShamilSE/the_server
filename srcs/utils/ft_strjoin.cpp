#include "../../includes/utils.hpp"

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