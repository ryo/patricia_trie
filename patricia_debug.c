#include <stdio.h>
#include "patricia_trie.h"

static void patricia_dump0(struct patricia_trie_node *, int);
static void patricia_debug0(struct patricia_trie_node *);


void
patricia_dump(struct patricia_trie *trieroot, int level)
{
	patricia_dump0(trieroot->root, level);
}

void
patricia_debug(struct patricia_trie *trieroot)
{
	patricia_debug0(trieroot->root);
}

static void
patricia_dump0(struct patricia_trie_node *trie, int level)
{
	int i, j;

#define	OUTPUT_INDENT()				\
	do {					\
		for (j = 0; j < level * 5; j++)	\
			printf(" ");		\
	} while (0)

	if (level == 0) {
		printf("===========================================\n");
	}
	OUTPUT_INDENT(); printf("struct patricia_trie_node (ptr=%p) {\n", trie);
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
				patricia_dump0((struct patricia_trie_node *)trie->pt_ptr[i], level + 1);

				OUTPUT_INDENT();
				printf("  }\n");
			}
		}
	}
	OUTPUT_INDENT(); printf("}\n");
}

static void
patricia_debug0(struct patricia_trie_node *trie)
{
	int i;
	int nlink;

	nlink = 0;
	for (i = 0; i < PATRICIATRIE_LINKNUM; i++) {
		if (trie->pt_ptr[i] != NULL) {
			nlink++;

			if (!TRIE_PTR_ISNODE(trie, i)) {
				patricia_debug0((struct patricia_trie_node *)trie->pt_ptr[i]);
			}
		}
	}
	if (nlink != trie->pt_nlink) {
		printf("WARNING: trie=%p, trie->pt_nlink = %d, but number of valid link = %d\n", trie, trie->pt_nlink, nlink);
	}
}
