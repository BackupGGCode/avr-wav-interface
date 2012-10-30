/*
 *
 */


#include "mix.h"

BYTE mix_audio8bit(BYTE a, BYTE b)
{

	int res = (int)a + (int)b - 127;

	if (res <0) {
		return (BYTE)0;
	}
	if (res > 255) {
		return (BYTE)255;
	}
	return (BYTE)res;

}
