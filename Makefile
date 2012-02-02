#	$Id: Makefile,v 1.8 2012/02/02 06:59:00 ryo Exp $

PROG=	patricia_test
SRCS=	patricia_trie_u.c patricia_debug.c test.c benchmark.c

NOMAN=	yes

LDADD+=	
DPADD+=	

.include <bsd.prog.mk>


benchmark:
	./patricia_test /usr/share/dict/web2
