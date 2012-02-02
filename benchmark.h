#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <sys/time.h>

typedef struct benchmark {
	struct timespec t_start;
	struct timespec t_end;
	struct timespec t_total;
} benchmark_t;

void benchmark_init(benchmark_t *);
void benchmark_start(benchmark_t *);
void benchmark_stop(benchmark_t *);
void benchmark_cont(benchmark_t *);
void benchmark_result(benchmark_t *);

#endif /* _BENCHMARK_H_ */
