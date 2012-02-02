#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static inline void *
xxx_malloc(size_t size)
{
	void *p;

	p = malloc(size);
	memset(p, 0, size);	/* for M_ZERO */

//	printf("MALLOC(%lu) -> %p\n", (unsigned long)size, p);

	return p;
}

#define	malloc(s, t, f)	xxx_malloc(s)
#define	KASSERT(e)	assert(e)
#define	free(p, t)	do { /*printf("FREE(%p)\n", p);*/ free(p); } while (0)


#include "patricia_trie.c"
