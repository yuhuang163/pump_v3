#include "stdio_impl.h"
#include <string.h>

int __fputs(const char *restrict s, FILE *restrict f)
{
	size_t l = strlen(s);
	return (fwrite(s, 1, l, f)==l) - 1;
}

weak_alias(__fputs, fputs_unlocked);

int WRAP(fputs)(const char *restrict s, FILE *restrict f)
{
	return __fputs(s, f);
}
