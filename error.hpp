#ifndef MY_ERROR_HPP
#define MY_ERROR_HPP

#include <errno.h>
#include <string.h>
#include <stdlib.h>

const unsigned int ENEXT2       = 200;
const unsigned int EBADINODENUM = 201;

const unsigned int MY_ERR_CODE[] =
{
	ENEXT2,
	EBADINODENUM
};

const char *const MY_ERR_STR[] =
{
	"file is not an ext2 fylesystem",
	"no suh node in file"
};

const unsigned int MY_ERR_SIZE = sizeof(MY_ERR_CODE) / sizeof(unsigned int);

#define RESET   "\033[0m"
#define RED     "\033[1;31m"

#define ERROR(message)                                     \
  printf("%sERROR! %s in file %s function %s line %d\n%s", \
  RED, message, __FILE__, __FUNCTION__, __LINE__, RESET)


#define ERROR_CHECK(error_condition)                                       \
  if (error_condition)                                                     \
  {                                                                        \
    printf("%sFATAL ERROR! %s occured in file %s function %s line %d\n%s", \
    RED, strerror(errno), __FILE__, __FUNCTION__, __LINE__, RESET);        \
    exit(EXIT_FAILURE);                                                    \
  }

#define ERRNO_MSG(num)                                         \
	{                                                            \
		unsigned int my_err = My_err_pos(num);                     \
		if (my_err + 1)                                            \
			fprintf(stderr, "%s:%s():%d: error: %s\n",               \
		        __FILE__, __func__, __LINE__, MY_ERR_STR[my_err]); \
		else                                                       \
			fprintf(stderr, "%s:%s():%d: error: %s\n",               \
		        __FILE__, __func__, __LINE__, strerror(num));      \
	}

int My_err_pos(int num);

#endif // MY_ERROR_HPP