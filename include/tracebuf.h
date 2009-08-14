#define TRACEBUF_SIZE 4096

typedef struct {
	uint8_t data[TRACEBUF_SIZE];
	int start;
	unsigned int end;
} TraceBuf;

inline static void tracebuf_init(TraceBuf *trace)
{
	trace->start = -1;
	trace->end = 0;
}

inline static void tracebuf_append(TraceBuf *trace, const uint8_t *data, unsigned int size)
{
	unsigned int end_space = TRACEBUF_SIZE - trace->end;

	trace->start = trace->end;

	if (size > TRACEBUF_SIZE) {
		data += size - TRACEBUF_SIZE;
		size = TRACEBUF_SIZE;
	}

	if (size <= end_space) {
		memcpy(trace->data + trace->end, data, size);
		trace->end += size;
	} else {
		memcpy(trace->data + trace->end, data, end_space);
		data += end_space;
		size -= end_space;
		memcpy(trace->data, data, size);
		trace->end = size;
	}
}

inline static bool tracebuf_empty(const TraceBuf *trace)
{
	return trace->start < 0;
}

inline static void tracebuf_dump(const TraceBuf *trace, char *path, size_t pathsize)
{
	mode_t mask;
	int fd;

	if (tracebuf_empty(trace))
		goto clear;

	mask = umask(0);
	mkdir("/tmp/pgbouncer-trace", 0777);
	umask(mask);

	strncpy(path, "/tmp/pgbouncer-trace/XXXXXX", pathsize);
	fd = mkstemp(path);
	if (fd < 0)
		goto clear;

	if ((unsigned int) trace->start < trace->end) {
		write(fd, trace->data + trace->start, trace->end - trace->start);
	} else {
		write(fd, trace->data + trace->start, TRACEBUF_SIZE - trace->start);
		write(fd, trace->data, trace->end);
	}

	fchmod(fd, 0666);
	close(fd);

	return;

clear:
	path[0] = '\0';
}
