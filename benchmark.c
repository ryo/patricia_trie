#include <stdio.h>
#include <string.h>

#include "benchmark.h"

void
benchmark_init(benchmark_t *benchmark)
{
	memset(benchmark, 0, sizeof(*benchmark));
}

void
benchmark_start(benchmark_t *benchmark)
{
	benchmark_init(benchmark);
	clock_gettime(CLOCK_MONOTONIC, &benchmark->t_start);
}

void
benchmark_stop(benchmark_t *benchmark)
{
	struct timespec t_sum, t_total;

	clock_gettime(CLOCK_MONOTONIC, &benchmark->t_end);

	timespecsub(&benchmark->t_end, &benchmark->t_start, &t_sum);
	timespecadd(&t_sum, &benchmark->t_total, &t_total);
	benchmark->t_total = t_total;
}

void
benchmark_cont(benchmark_t *benchmark)
{
	clock_gettime(CLOCK_MONOTONIC, &benchmark->t_start);
}

void
benchmark_result(benchmark_t *benchmark)
{
	printf("%lu.%09lu sec\n",
	    (unsigned long)benchmark->t_total.tv_sec,
	    (unsigned long)benchmark->t_total.tv_nsec);
}


