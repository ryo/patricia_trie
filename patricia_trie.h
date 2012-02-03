/*	$Id: patricia_trie.h,v 1.9 2012/02/03 07:08:29 ryo Exp $	*/
/*-
 * Copyright (c) 2011 SHIMIZU Ryo <ryo@nerv.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PATRICIA_TRIE_H_
#define _PATRICIA_TRIE_H_

#include <sys/types.h>
#include "patricia_config.h"

struct patricia_trie {
	struct patricia_trie_node *root;
};

struct patricia_trie_node {
	struct patricia_trie_node *pt_parent;
	uint16_t pt_parentidx;
	uint16_t pt_nlink;
	void *pt_ptr[PATRICIATRIE_LINKNUM];	/* value pointer or branch */
	uint32_t pt_ptr_isnode[PATRICIATRIE_LINKNUM / 32];
	char pt_match[PATRICIATRIE_STRINGMAX];
};

#define	TRIE_PTR_ISNODE(trie, n)	\
	(((trie)->pt_ptr_isnode)[(n) / 32] & (1 << ((n) & 31)))
#define	TRIE_PTR_SETNODE(trie, n)	\
	(((trie)->pt_ptr_isnode)[(n) / 32] |= (1 << ((n) & 31)))
#define	TRIE_PTR_SETBRANCH(trie, n)	\
	(((trie)->pt_ptr_isnode)[(n) / 32] &= ~(1 << ((n) & 31)))

struct patricia_trie *patricia_new(void);
void *patricia_insert(struct patricia_trie *, const char *, void *);
void *patricia_set(struct patricia_trie *, const char *, void *);
int patricia_delete(struct patricia_trie *, const char *);
void *patricia_find(struct patricia_trie *, const char *);
void patricia_dump(struct patricia_trie *, int);
void patricia_debug(struct patricia_trie *);

#endif /* _PATRICIA_TRIE_H_ */
