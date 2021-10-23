#include <errno.h>
#include <string.h>

#define ERROR(message)                                          \
  printf("ERROR! %s in file %s function %s line %d\n",          \
  message, __FILE__, __FUNCTION__, __LINE__)


#define ERROR_CHECKING(task)                                    \
  errno = 0;                                                    \
  task;                                                         \
  if (errno != 0)                                               \
  {                                                             \
    printf("ERROR! %s occured in file %s function %s line %d\n", \
    strerror(errno), __FILE__, __FUNCTION__, __LINE__);         \
  }

