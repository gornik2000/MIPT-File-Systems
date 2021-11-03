#include "error.hpp"

int My_err_pos(int num)
{
	for (int i = 0; i < MY_ERR_SIZE; ++i)
		if (MY_ERR_CODE[i] == num) 
			return i;

	return -1;
}