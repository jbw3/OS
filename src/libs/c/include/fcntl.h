#ifndef _FCNTL_H
#define _FCNTL_H 1

#define O_RDONLY (0x01)
#define O_WRONLY (0x02)
#define O_RDWR   (O_RDONLY | O_WRONLY)
#define O_EXEC   (0x04)
#define O_SEARCH (0x04)

#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR | O_EXEC | O_SEARCH)

#ifdef __cplusplus
extern "C"
{
#endif

int open(const char* path, int oflag, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _FCNTL_H */
