#include "pipe.h"

#include "../utils.h"

#include <string.h>
#include <errno.h>

/*
 * @see header
 */
int pipe_init(struct pipe* pipe) {
	int ret;

	ret = sem_init(&pipe->write, 0, 0);
	if (ret)
		goto err;
	ret = sem_init(&pipe->read, 0, 0);
	if (ret)
		goto err;

	return 0;

err:
	return -EIO;
}

/*
 * @see header
 */
void pipe_destroy(struct pipe* pipe) {
	sem_destroy(&pipe->write);
	sem_destroy(&pipe->read);
}

/*
 * @see header
 */
ssize_t pipe_write(struct pipe* pipe, void *data, size_t len) {
	sem_wait(&pipe->read);

	if (len < pipe->len) {
		pipe->len = len;
	}
	memcpy(pipe->buf, data, pipe->len);

	sem_post(&pipe->write);

	return pipe->len;
}

/*
 * @see header
 */
ssize_t pipe_writereq(struct pipe *pipe, size_t len) {
	sem_wait(&pipe->read);

	if (len < pipe->len) {
		pipe->len = len;
	}

	return pipe->len;
}

/*
 * @see header
 */
void pipe_writeareq(struct pipe *pipe, void *data) {
	memcpy(pipe->buf, data, pipe->len);

	sem_post(&pipe->write);
}

/*
 * @see header
 */
ssize_t pipe_read(struct pipe* pipe, void *data, size_t len) {
	pipe->len = len;
	pipe->buf = data;

	sem_post(&pipe->read);
	sem_wait(&pipe->write);

	return pipe->len;
}