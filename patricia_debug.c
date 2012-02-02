#include <stdio.h>
#include "patricia_trie.h"

void
patricia_dump(struct patricia_trie **trie_p, int level)
{
	struct patricia_trie *trie;
	int i, j;

	trie = *trie_p;

#define	OUTPUT_INDENT()				\
	do {					\
		for (j = 0; j < level * 5; j++)	\
			printf(" ");		\
	} while (0)

	if (level == 0) {
		printf("===========================================\n");
	}
	OUTPUT_INDENT(); printf("struct patricia_trie (ptr=%p) {\n", trie);
	OUTPUT_INDENT(); printf("  .pt_parent = %p\n", trie->pt_parent);
	OUTPUT_INDENT(); printf("  .pt_parentidx = 0x%x\n", trie->pt_parentidx);
	OUTPUT_INDENT(); printf("  .pt_nlink = %d\n", trie->pt_nlink);
	OUTPUT_INDENT(); printf("  .pt_match = \"%s\"\n", trie->pt_match);

	for (i = 0; i < PATRICIATRIE_LINKNUM; i++) {
		if (trie->pt_ptr[i] != NULL) {
			OUTPUT_INDENT();
			if (TRIE_PTR_ISNODE(trie, i)) {
				printf("  .pt_ptr[0x%02x('%c')] = %p (VALUE)\n", i, i, trie->pt_ptr[i]);
			} else {
				printf("  .pt_ptr[0x%02x('%c')] = %p => {\n", i, i, trie->pt_ptr[i]);
				patricia_dump((struct patricia_trie **)&trie->pt_ptr[i], level + 1);

				OUTPUT_INDENT();
				printf("  }\n");
			}
		}
	}
	OUTPUT_INDENT(); printf("}\n");
}

void
patricia_debug(struct patricia_trie **trie_p)
{
	struct patricia_trie *trie;
	int i;
	int nlink;

	trie = *trie_p;

	nlink = 0;
	for (i = 0; i < PATRICIATRIE_LINKNUM; i++) {
		if (trie->pt_ptr[i] != NULL) {
			nlink++;

			if (!TRIE_PTR_ISNODE(trie, i)) {
				patricia_debug((struct patricia_trie **)&trie->pt_ptr[i]);
			}
		}
	}
	if (nlink != trie->pt_nlink) {
		printf("WARNING: trie=%p, trie->pt_nlink = %d, but number of valid link = %d\n", trie, trie->pt_nlink, nlink);
	}
}
