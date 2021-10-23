#include <errno.h>
#include <string.h>

#define RESET   "\033[0m"
#define RED     "\033[1;31m"

#define ERROR(message)                                               \
  printf("%sERROR! %s in file %s function %s line %d\n%s",           \
  RED, message, __FILE__, __FUNCTION__, __LINE__, RESET)


#define ERROR_CHECK(error_condition)                                 \
  if (error_condition)                                               \
  {                                                                  \
    printf("%sERROR! %s occured in file %s function %s line %d\n%s", \
    RED, strerror(errno), __FILE__, __FUNCTION__, __LINE__, RESET);  \
  }

