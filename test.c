#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <time.h>
#include "patricia_trie.h"
#include "benchmark.h"

extern char *optarg;
extern int optind;

static void usage(void);

static void
usage()
{
	printf("usage: patricia_test [options] [file]\n");
	printf("	-n <nloop>	specify benchmark loop count\n");
	printf("	-d		dump patricia trie\n");
	printf("	-v		verbose\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	size_t size;
	struct patricia_trie *trieroot;
	FILE *fh;
	char *file, *filebuf, *line, *p;
	void *result;
	int ch, dflag, vflag;
	int i, nloop;
	unsigned int lineno, nlines;
	benchmark_t bench_insert, bench_find, bench_delete;

	dflag = vflag = 0;
	nloop = 1;
	while ((ch = getopt(argc, argv, "dn:v")) != -1) {
		switch (ch) {
		case 'd':
			dflag++;
			break;
		case 'n':
			nloop = atoi(optarg);
			break;
		case 'v':
			vflag++;
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	file = argv[0];

	/*
	 * read file on memory
	 */
	fh = fopen(file, "r");
	if (fh == NULL)
		errx(1, "open: %s", file);
	fseek(fh, 0, SEEK_END);
	size = ftell(fh);

	filebuf = malloc(size);
	if (filebuf == NULL) {
		fprintf(stderr, "cannot allocate memory\n");
		exit(1);
	}
	rewind(fh);
	fread(filebuf, size, 1, fh);
	fclose(fh);

	/* replace from '\n' to '\0' */
	for (nlines = 0, p = filebuf; p < filebuf + size; p++) {
		if (*p == '\n') {
			*p = '\0';
			nlines++;
		}
	}
	/*
	 * now, filebuf is "A\0a\0aa\0aal\0aalii\0aam\0Aani\0aardvark\0".....
	 */

	benchmark_start(&bench_insert);
	benchmark_start(&bench_find);
	benchmark_start(&bench_delete);


	trieroot = patricia_new();

	for (i = 0; i < nloop; i++) {
		benchmark_cont(&bench_insert);
		for (lineno = 1, line = filebuf; line < filebuf + size;
		    line += strlen(line) + 1, lineno++) {
			if (patricia_insert(trieroot, line, (void *)lineno) == NULL) {
				fprintf(stderr, "patricia_insert: DUPLICATE KEY: <%s>\n", line);
			}
		}
		benchmark_stop(&bench_insert);
		if (vflag || (i + 1 == nloop)) {
			printf("insert: %d lines, %d loop: ", lineno - 1, i + 1);
			benchmark_result(&bench_insert);
		}

		if (dflag >= 2)
			patricia_dump(trieroot, 0);

		benchmark_cont(&bench_find);
		for (lineno = 1, line = filebuf; line < filebuf + size;
		    line += strlen(line) + 1, lineno++) {

			result = patricia_find(trieroot, line);
			if (result != (void *)lineno) {
				fprintf(stderr, "patricia_find: ERROR: key <%s> has inserted %d, but %d\n", line, lineno, (int)result);
			}
		}
		benchmark_stop(&bench_find);
		if (vflag || (i + 1 == nloop)) {
			printf("find:   %d lines, %d loop: ", lineno - 1, i + 1);
			benchmark_result(&bench_find);
		}

		benchmark_cont(&bench_delete);
		for (lineno = 1, line = filebuf; line < filebuf + size;
		    line += strlen(line) + 1, lineno++) {
			patricia_delete(trieroot, line);
		}
		benchmark_stop(&bench_delete);
		if (vflag || (i + 1 == nloop)) {
			printf("delete: %d lines, %d loop: ", lineno - 1, i + 1);
			benchmark_result(&bench_delete);
		}

		if (dflag)
			patricia_dump(trieroot, 0);

		if (vflag)
			printf("\n");
	}
}
