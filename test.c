#include "io.h"
#include <bits/time.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>

const size_t KB = 1024;
const size_t MB = KB * 1024L;
const size_t GB = MB * 1024L;
const size_t BUF_SIZE = 1 * MB / 4;
const size_t FILE_SIZE = 12 * GB;
const int N = 5;
char filename[] = "big_test_file";

double compute_time(struct timespec start, struct timespec end) {
	double nano = (double)(end.tv_nsec - start.tv_nsec);
	double sec = (double)((end.tv_sec - start.tv_sec) * 1E9);
	return (sec + nano) / (1E9);
}

double write_test(int direct) {
	int out_fd = -1;
	char *buffer = aligned_malloc(BUF_SIZE);
	char *tmp = buffer;
	size_t bytes_written = 0;
	size_t total_bytes_written = 0;
	struct timespec start, end;

	int flags = O_RDWR | O_CREAT | O_TRUNC;
	if (direct)
		flags |= __O_DIRECT;

	out_fd = open(filename, flags, 0600);
	if (out_fd < 0)
		return -1;
	memset(buffer, 0, BUF_SIZE);
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	while (total_bytes_written < FILE_SIZE) {
		bytes_written = write_file(out_fd, buffer, BUF_SIZE);
		if (bytes_written < 0)
			return -1;
		total_bytes_written += bytes_written;
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);

	free(buffer);
	close(out_fd);

	double write_dur = compute_time(start, end);
	printf("bytes written= %ld\twrite time= %f sec\n", total_bytes_written,
		   write_dur);

	return write_dur;
}

double read_test(int direct) {
	int in_fd = -1;
	char *buffer = aligned_malloc(BUF_SIZE);
	size_t bytes_read = 0;
	size_t total_bytes_read = 0;
	struct timespec start, end;

	int flags = O_RDONLY | O_SYNC;
	if (direct)
		flags |= __O_DIRECT;

	in_fd = open(filename, flags, 0600);
	if (in_fd < 0)
		return -1;
	memset(buffer, 0, BUF_SIZE);
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	while (total_bytes_read < FILE_SIZE) {
		bytes_read = read(in_fd, buffer, BUF_SIZE);
		if (bytes_read < 0)
			return -1;
		total_bytes_read += bytes_read;
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);

	free(buffer);
	close(in_fd);

	double read_dur = compute_time(start, end);
	printf("bytes read= %ld\tread time = %f sec\n", total_bytes_read, read_dur);

	return read_dur;
}

int run_test(int direct) {
	double write_dur, read_dur;
	double total_write_dur = 0, total_read_dur = 0;

	for (int i = 0; i < N; i++) {
		printf("----RUN %d----\n", i + 1);
		write_dur = write_test(direct);
		read_dur = read_test(direct);
		remove(filename);

		total_read_dur += read_dur;
		total_write_dur += write_dur;
	}

	printf("--------------\n");

	float avg_write_time = total_write_dur / N;
	float avg_write_speed = (float)FILE_SIZE / avg_write_time / (1024 * 1024);
	printf("AVG WRITE TIME = %f sec\t AVG WRITE SPEED = %f MB/s\n",
		   avg_write_time, avg_write_speed);

	float avg_read_time = total_read_dur / N;
	float avg_read_speed = (float)FILE_SIZE / avg_read_time / (1024 * 1024);
	printf("AVG READ TIME = %f sec\t AVG READ SPEED = %f MB/s\n", avg_read_time,
		   avg_read_speed);
}

int main(int argc, char **argv) {
	int in_fd = -1;
	size_t bytes_read = 0;
	size_t total_bytes_read = 0;
	float read_dur = 0;
	float total_read_dur = 0;

	printf("****DIRECT-IO TEST****\n");
	printf("file size = %luGB, buffer size = %luKB\n", FILE_SIZE / GB,
		   BUF_SIZE / KB);
	run_test(1);
	printf("**********************\n");

	printf("\n\n****BUFFERED-IO TEST****\n");
	printf("file size = %luGB, buffer size = %luKB\n", FILE_SIZE / GB,
		   BUF_SIZE / KB);
	run_test(0);
	printf("**********************\n");
}