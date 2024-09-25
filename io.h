#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#define BLOCK_SIZE 512

/**
 * Wrapper around write() libc call
 * @param fd The file descriptor
 * @param buffer The buffer to be written
 * @param bytes The number of bytes to be written
 * @return The number of bytes written, -1 on error
 */
ssize_t write_file(int fd, void *buffer, size_t bytes);

/**
 * Wrapper around aligned_alloc() libc call
 * @param size The size of the buffer
 */
void *aligned_malloc(size_t size);
