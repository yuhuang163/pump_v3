#include "stdio_impl.h"

#undef feof

int __feof(FILE *f)
{
	FLOCK(f);
	int ret = !!(f->flags & F_EOF);
	FUNLOCK(f);
	return ret;
}

weak_alias(__feof, feof_unlocked);
weak_alias(__feof, _IO_feof_unlocked);

int WRAP(feof)(FILE *f)
{
	return __feof(f);
}
