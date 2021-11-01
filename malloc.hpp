#ifndef _MY_XMALLOC

#include <stdlib.h>
#include <string.h>

#define BUG_ON(condition, action) if (condition) action;

#define OOM()                                                   \
	do                                                            \
	{                                                             \
		fprintf(stderr, "%s:%s():%d: Fatal error: Out of memory\n", \
		        __FILE__, __func__, __LINE__);                      \
		exit(EXIT_FAILURE);                                         \
	}                                                             \
	while (0);

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

#define _MY_XMALLOC value
#endif