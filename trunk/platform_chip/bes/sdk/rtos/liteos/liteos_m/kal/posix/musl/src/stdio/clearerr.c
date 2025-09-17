#include "stdio_impl.h"

void __clearerr(FILE *f)
{
	FLOCK(f);
	f->flags &= ~(F_EOF|F_ERR);
	FUNLOCK(f);
}

weak_alias(__clearerr, clearerr_unlocked);

void WRAP(clearerr)(FILE *f)
{
	__clearerr(f);
}
