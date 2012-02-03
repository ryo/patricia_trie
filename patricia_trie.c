/*	$Id: patricia_trie.c,v 1.14 2012/02/03 07:08:29 ryo Exp $	*/
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

#include "patricia_trie.h"

static struct patricia_trie_node *patricia_newbranch(const char *, void *);
static void *patricia_insert0(struct patricia_trie_node **, const char *, void *,
                              int);
static void **patricia_find0(struct patricia_trie_node *, const char *,
                             struct patricia_trie_node **);
static void patricia_cleanup(struct patricia_trie_node *);

struct patricia_trie *
patricia_new()
{
	struct patricia_trie *trieroot;
	struct patricia_trie_node *trienode;

	trieroot = malloc(sizeof(struct patricia_trie), M_TEMP,
	    M_WAITOK | M_ZERO);
	if (trieroot == NULL)
		return NULL;

	trienode = patricia_newbranch(NULL, NULL);
	if (trienode == NULL) {
		free(trieroot, M_TEMP);
		return NULL;
	}
	trienode->pt_parent = NULL;
	trieroot->root = trienode;

	return trieroot;
}

static struct patricia_trie_node *
patricia_newbranch(const char *key, void *value)
{
	struct patricia_trie_node *trie;
	char *matchp;
	unsigned char c;

	trie = malloc(sizeof(struct patricia_trie_node), M_TEMP,
	    M_WAITOK | M_ZERO);

	if (key != NULL) {
		/*
		 * same as strcpy(trie->pt_match, key),
		 * but for getting last character of key
		 */
		for (matchp = trie->pt_match;;) {
			c = *key++;
			if ((c == '\0') || (*key == '\0'))
				break;
			*matchp++ = c;
		}

		/* <c> is last character of <key> */
		KASSERT(c < PATRICIATRIE_LINKNUM);

		*matchp = '\0';
		trie->pt_ptr[c] = value;
		trie->pt_nlink = 1;
		TRIE_PTR_SETNODE(trie, c);
	}

	return trie;
}

void *
patricia_insert(struct patricia_trie *trieroot, const char *key, void *value)
{
	return patricia_insert0(&trieroot->root, key, value, 0);
}

void *
patricia_set(struct patricia_trie *trieroot, const char *key, void *value)
{
	return patricia_insert0(&trieroot->root, key, value, 1);
}

static void *
patricia_insert0(struct patricia_trie_node **trie_p, const char *key, void *value, int force)
{
	struct patricia_trie_node *trie, *trie_tmp;
	const char *matchp;
	unsigned char c, idx1, idx2;

	trie = *trie_p;

 trie_add_next:
	for (matchp = trie->pt_match;;) {
		if ((c = *matchp++) == '\0') {

			c = *key++;
			KASSERT(c < PATRICIATRIE_LINKNUM);

			/* is entry available? */
			if (trie->pt_ptr[c] == NULL) {
				/* match full? or only 1 character left? */
				if ((c == '\0') || (*key == '\0')) {
					/*
					 * new node case.
					 * append node "key2"; c == '2'
					 *
					 * [OLD]
					 * {
					 *   pt_match = "key"
					 *   pt_ptr['1'] = <key1>
					 * }
					 *
					 * [NEW]
					 * {
					 *   pt_match = "key"
					 *   pt_ptr['1'] = <key1>
					 *   pt_ptr['2'] = <key2>
					 * }
					 */
					TRIE_PTR_SETNODE(trie, c);
					trie->pt_ptr[c] = value;
					trie->pt_nlink++;
					return trie->pt_ptr[c];
				}

				/*
				 * new tree case
				 * append node "key2048"; c == '2'
				 *
				 * [OLD]
				 * {
				 *   pt_match = "key"
				 *   pt_ptr['1'] = <key1>
				 * }
				 *
				 * [NEW]
				 * {
				 *   pt_match = "key"
				 *   pt_ptr['1'] = <key1>
				 *   pt_ptr['2'] = {
				 *      pt_match = "04"
				 *      pt_ptr['8'] = <key2048>
				 *   }
				 * }
				 */
				trie_tmp = patricia_newbranch(key, value);
				trie_tmp->pt_parent = trie;
				trie_tmp->pt_parentidx = c;

				TRIE_PTR_SETBRANCH(trie, c);
				trie->pt_ptr[c] = trie_tmp;
				trie->pt_nlink++;

				return trie_tmp;
			}

			/* this entry is unavailable */
			if (TRIE_PTR_ISNODE(trie, c)) {
				/* this entry used for node */

				if ((c == '\0') || (*key == '\0')) {
					/* duplicated key */
					if (!force)
						return NULL;

					/* override this key */
					TRIE_PTR_SETNODE(trie, c);
					trie->pt_ptr[c] = value;
					return trie->pt_ptr[c];
				}

				/*
				 * split node case.
				 * append node "key10"
				 *
				 * [OLD]
				 * {
				 *   pt_match = "key"
				 *   pt_ptr['1'] = <key1>
				 * }
				 *
				 * [NEW](in progress)
				 * {
				 *   pt_match = "key"
				 *   pt_ptr['1'] = {
				 *     pt_match = ""
				 *     pt_ptr['\0'] = <key1>
				 *   }
				 * }
				 */
				trie_tmp = patricia_newbranch("",
				    trie->pt_ptr[c]);
				trie_tmp->pt_parent = trie;
				trie_tmp->pt_parentidx = c;

				TRIE_PTR_SETBRANCH(trie, c);
				trie->pt_ptr[c] = trie_tmp;

				/* and continue to add "key10" */
			}
			trie_p = (struct patricia_trie_node **)&trie->pt_ptr[c];
			trie = trie->pt_ptr[c];
			goto trie_add_next;
		}

		/* no match. found collision */
		if (c != *key++)
			break;
	}

	/* backward one character */
	key -= 1;
	matchp -= 1;

	/*
	 * split tree case.
	 * append "keyabxyz0"
	 *
	 * [OLD]
	 * {
	 *   pt_match = "keyabcde"
	 *   pt_ptr['0'] = <keyabcde0>
	 *   pt_ptr['1'] = <keyabcde1>
	 * }
	 * 
	 * [NEW]
	 * {
	 *   pt_match = "keyab"
	 *   pt_ptr['c'] = {
	 *     pt_match = "de"
	 *     pt_ptr['0'] = <keyabcde0>
	 *     pt_ptr['1'] = <keyabcde1>
	 *   }
	 *   pt_ptr['x'] = {
	 *     pt_match = "yz"
	 *     pt_ptr['0'] = <keyabxyz0>
	 *   }
	 * }
	 */

	idx1 = *matchp;
	idx2 = *key;

	KASSERT(idx1 < PATRICIATRIE_LINKNUM);
	KASSERT(idx2 < PATRICIATRIE_LINKNUM);

	/*
	 * create branch, and rotate branch
	 */
	trie_tmp = patricia_newbranch(NULL, NULL);
	*trie_p = trie_tmp;
	trie_tmp->pt_parent = trie->pt_parent;
	trie_tmp->pt_parentidx = trie->pt_parentidx;
	trie_tmp->pt_ptr[idx1] = trie;
	trie->pt_parent = trie_tmp;
	trie->pt_parentidx = idx1;

	/* trie_tmp is new parent, and trie is child of trie_tmp */
	if (idx2 == '\0') {
		TRIE_PTR_SETNODE(trie_tmp, idx2);
		trie_tmp->pt_ptr[idx2] = value;
	} else {
		trie_tmp->pt_ptr[idx2] = patricia_newbranch(key + 1, value);
		((struct patricia_trie_node *)trie_tmp->pt_ptr[idx2])->pt_parent =
		    trie_tmp;
		((struct patricia_trie_node *)trie_tmp->pt_ptr[idx2])->pt_parentidx =
		    idx2;
	}

	trie_tmp->pt_nlink = 2;	/* idx1 and idx2 */

	memcpy(trie_tmp->pt_match, trie->pt_match,
	    matchp - trie->pt_match);
	trie_tmp->pt_match[matchp - trie->pt_match] = '\0';
	strncpy(trie->pt_match,
	    &trie->pt_match[matchp + 1 - trie->pt_match],
	    sizeof(trie->pt_match));
	trie = trie->pt_ptr[c];

	return trie_tmp->pt_ptr[c];
}

static void **
patricia_find0(struct patricia_trie_node *trie, const char *key,
               struct patricia_trie_node **trie_ref)
{
	const char *m;
	unsigned char c;
	void *found;

	for (;;) {
		for (m = trie->pt_match;;) {
			if ((c = *m++) == '\0')
				break;
			if (c != *key++)
				return NULL;
		}

		c = *key;
		KASSERT(c < PATRICIATRIE_LINKNUM);

		found = trie->pt_ptr[c];
		if (found == NULL)
			return NULL;

		if (TRIE_PTR_ISNODE(trie, c)) {
			if ((c != '\0') && (*++key != '\0'))
				return NULL;

			if (trie_ref != NULL)
				*trie_ref = trie;

			return &trie->pt_ptr[c];
		}

		++key;
		trie = found;
	}
}

static void
patricia_cleanup(struct patricia_trie_node *trie)
{
	struct patricia_trie_node *parent, *trie2;
	char *p;
	int i;

	/*
	 * free empty branch
	 */
	for (; trie->pt_nlink <= 1; trie = parent) {
		parent = trie->pt_parent;

		/* never free root */
		if (parent == NULL)
			return;

		if (trie->pt_nlink == 0) {
			KASSERT(parent->pt_ptr[trie->pt_parentidx] == trie);
			parent->pt_ptr[trie->pt_parentidx] = NULL;
			parent->pt_nlink--;

			free(trie, M_TEMP);

		} else if (trie->pt_nlink == 1) {
			/*
			 * join branch that has only 1 branch
			 */

			/* search index of the child */
			for (i = 0; i < PATRICIATRIE_LINKNUM; i++)
				if (trie->pt_ptr[i] != NULL)
					break;

			if (!TRIE_PTR_ISNODE(trie, i)) {
				trie2 = (struct patricia_trie_node *)trie->pt_ptr[i];

				for (p = trie->pt_match; *p != '\0'; p++)
					;
				*p++ = i;
				strcpy(p, trie2->pt_match);
				strcpy(trie2->pt_match, trie->pt_match);

				parent = trie->pt_parent;
				if (parent != NULL) {
					parent->pt_ptr[trie->pt_parentidx] =
					    trie2;
				}
				trie2->pt_parent = parent;
				trie2->pt_parentidx = trie->pt_parentidx;

				free(trie, M_TEMP);
			}
			break;
		}
	}
}

int
patricia_delete(struct patricia_trie *trieroot, const char *key)
{
	struct patricia_trie_node *trie;
	void **ref;

	ref = patricia_find0(trieroot->root, key, &trie);
	if (ref == NULL)
		return -1;

	*ref = NULL;
	trie->pt_nlink--;

	patricia_cleanup(trie);

	return 0;
}

void *
patricia_find(struct patricia_trie *trieroot, const char *key)
{
	void **ref;

	ref = patricia_find0(trieroot->root, key, NULL);
	if (ref == NULL)
		return NULL;

	return *ref;
}
