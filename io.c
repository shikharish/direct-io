#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLOCK_SIZE 512
#define BUF_SIZE 512

ssize_t write_file(void *buffer, size_t bytes, int fd) {
	ssize_t bytes_written = 0;
	if (bytes % BLOCK_SIZE != 0) {
		int flags = fcntl(fd, F_GETFL);
		if (fcntl(fd, F_SETFL, flags & ~__O_DIRECT)) {
			printf("failed to disable O_DIRECT\n");
		}

		bytes_written = write(fd, buffer, bytes);

		if (fcntl(fd, F_SETFL, flags)) {
			printf("failed to re-enable O_DIRECT\n");
		}
	} else {
		bytes_written = write(fd, buffer, bytes);
	}

	if (bytes_written < 0) {
		printf("error writing to fd: %d\n", fd);
		return -1;
	}

	printf("write: +%ld bytes fd: %d\n", bytes_written, fd);
	return bytes_written;
}

void *malloc_direct(size_t size) { return aligned_alloc(BLOCK_SIZE, size); }

int main() {
	char in_path[100];
	printf("input file: ");
	scanf("%s", in_path);
	int in_fd = open(in_path, O_RDONLY | __O_DIRECT);
	if (in_fd < 0) {
		printf("error opening input file\n");
		return 1;
	}

	size_t buf_len = BUF_SIZE;
	char *buf = malloc_direct(buf_len);

	char out_path[100];
	printf("output file: ");
	scanf("%s", out_path);
	int out_fd = open(out_path, O_RDWR | O_CREAT | O_TRUNC | __O_DIRECT, 0666);
	if (out_fd < 0) {
		printf("error opening output file\n");
		return 1;
	}

	ssize_t bytes_read = 0;
	ssize_t total_bytes_read = 0;
	ssize_t bytes_written = 0;
	ssize_t total_bytes_written = 0;

	while ((bytes_read = read(in_fd, buf, buf_len)) > 0) {
		total_bytes_read += bytes_read;
		bytes_written = write_file(buf, bytes_read, out_fd);
		if (bytes_written < 0) {
			return 1;
		}
		total_bytes_written += bytes_written;
	}

	printf("total bytes read = %ld\n", total_bytes_read);
	printf("total bytes written = %ld\n", total_bytes_written);

	close(in_fd);
	close(out_fd);
}
