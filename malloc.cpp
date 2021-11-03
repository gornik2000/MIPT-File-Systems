#include "malloc.hpp"

void *xmalloc(size_t size)
{
	void *ret = malloc(size);
	BUG_ON(ret == 0, OOM());

	return ret;
}

void *xzmalloc(size_t size)
{
	void *ret = malloc(size);
	BUG_ON(ret == 0, OOM());

	memset(ret, 0, size);
	return ret;
}