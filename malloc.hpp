#ifndef MY_MALLOC_HPP
#define MY_MALLOC_HPP

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUG_ON(condition, action) if (condition) action;

#define OOM()                                                   \
	do                                                            \
	{                                                             \
		fprintf(stderr, "%s:%s():%d: Fatal error: Out of memory\n", \
		        __FILE__, __func__, __LINE__);                      \
		exit(EXIT_FAILURE);                                         \
	}                                                             \
	while (0);

void *xmalloc(size_t size);
void *xzmalloc(size_t size);

#endif // MY_MALLOC_HPP