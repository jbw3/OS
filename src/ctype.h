#ifndef _CTYPE_H
#define _CTYPE_H 1

#ifdef __cplusplus
extern "C"
{
#endif

int isspace(int ch);

int isalpha(int ch);

int isupper(int ch);

int islower(int ch);

int isdigit(int ch);

int isxdigit(int ch);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _CTYPE_H */
