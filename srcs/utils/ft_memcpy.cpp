#include "../../includes/utils.hpp"

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
