#include "io.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ssize_t write_file(int fd, void *buffer, size_t bytes) {
	ssize_t bytes_written = 0;
	int flags = fcntl(fd, F_GETFL);
	int misalignment = (uintptr_t)buffer % BLOCK_SIZE;
	int odd_bytes = bytes % BLOCK_SIZE;

	if (flags & __O_DIRECT) {
		if (misalignment != 0) {
			printf("unaligned buffer!\n");
			void *aligned_buf =
				aligned_malloc(((bytes) / BLOCK_SIZE + 1) * BLOCK_SIZE);
			memcpy(aligned_buf, buffer, bytes);
			buffer = aligned_buf;
		}

		if (odd_bytes != 0) {
			char *tmp_buf = (char *)buffer;
			while (bytes_written + 8192 < bytes) {
				bytes_written += write(fd, tmp_buf + bytes_written, 8192);
			}
			if (fcntl(fd, F_SETFL, flags & ~__O_DIRECT)) {
				printf("Failed to disable O_DIRECT\n");
				return -1;
			}

			bytes_written +=
				write(fd, tmp_buf + bytes_written, bytes - bytes_written);

			if (fcntl(fd, F_SETFL, flags)) {
				printf("Failed to re-enable O_DIRECT\n");
				return -1;
			}
		} else {
			bytes_written = write(fd, buffer, bytes);
		}

	} else {
		bytes_written = write(fd, buffer, bytes);
	}

	if (bytes_written < 0) {
		printf("Error writing to fd=%d : %s\n", fd, strerror(errno));
		errno = 0;
		return -1;
	}

	// printf("write: +%ld bytes fd: %d\n", bytes_written, fd);
	return bytes_written;
}

void *aligned_malloc(size_t size) { return aligned_alloc(BLOCK_SIZE, size); }
