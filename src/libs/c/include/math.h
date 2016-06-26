#ifndef _MATH_H
#define _MATH_H 1

#define M_E        2.718281828459045235360287471353
#define M_LOG2E    1.442695040888963407387651782798
#define M_LOG10E   0.434294481903251827672584467477
#define M_LN2      0.693147180559945309428690474185
#define M_LN10     2.302585092994045684036338861311
#define M_PI       3.141592653589793238462643383280
#define M_PI_2     1.570796326794896619256404479703
#define M_PI_4     0.785398163397448309628202239852
#define M_1_PI     0.318309886183790671538174771316
#define M_2_PI     0.636619772367581343076349542631
#define M_2_SQRTPI 1.128379167095512573847949922001
#define M_SQRT2    1.414213562373095048801688724210
#define M_SQRT1_2  0.707106781186547524436104145140

#define isnan(n) \
( \
     sizeof(n) == sizeof(float) ? isnan_f(n) : \
     sizeof(n) == sizeof(double) ? isnan_d(n) : \
     isnan_ld(n) \
)

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************
 Implementation specific
***************************************/

int isnan_f(float n);

int isnan_d(double n);

int isnan_ld(long double n);

/***************************************
 Standard library
***************************************/

// TODO

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MATH_H */
