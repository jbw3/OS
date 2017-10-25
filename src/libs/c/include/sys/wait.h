#ifndef _WAIT_H
#define _WAIT_H 1

#define WCONTINUED (0x1)
#define WNOHANG    (0x2)
#define WUNTRACED  (0x4)

typedef int pid_t;

#ifdef __cplusplus
extern "C"
{
#endif

pid_t wait(int* stat_loc);

pid_t waitpid(pid_t pid, int* stat_loc, int options);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _WAIT_H */
