#include <errno.h>
#include <string.h>
#include <stdlib.h>

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

#define ERRNO_MSG()                                         \
	{                                                         \
		int errno_saved = errno;                                \
		fprintf(stderr, "%s:%s():%d: error: %s\n",              \
		        __FILE__, __func__, __LINE__, strerror(errno)); \
		errno = errno_saved;                                    \
	}
