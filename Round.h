#ifndef _INC_ROUND
#define _INC_ROUND

// Fast Round in Assembler, this is faster than truncation (=cast to int)!
__forceinline int Round(const double num)
{
	int n;
	__asm fld num;
	__asm fistp n;
	return n;
}
__forceinline int RoundF(const float num)
{
	int n;
	__asm fld num;
	__asm fistp n;
	return n;
}

// Slow
/*
__forceinline int Round(double num)
{
	return (num < 0.0 ? (int)(num - 0.5) : (int)(num + 0.5));
}

or

#define round(x) ((x)>=0)?(int)((x)+0.5):(int)((x)-0.5)
*/

#endif