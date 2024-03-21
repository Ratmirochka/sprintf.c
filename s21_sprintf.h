#ifndef S21_STRING_SPRINTF_H
#define S21_STRING_SPRINTF_H

#include <math.h>

#include "s21_string.h"
#include "s21_string_internal.h"

typedef struct flags {
  int width;
  int minus;
  int plus;
  int space;
  int zero;
  int hashtag;
  int precision;
  int is_precision;
  int length;
  int specifier;
  int firstLoop;
} flg;

void get_flags(const char **form, flg *f);
void get_width(const char **form, flg *f, va_list va);
int s21_parse_flg(char *tmp);
int s21_isdigit(char c);
void get_precision(const char **form, flg *f, va_list va);
void get_length(const char **form, flg *f);
char *s21_args_value(char *str, flg f, va_list va, char *str_start);
char *s21_parse_int(char *str, va_list va, flg f);
char *s21_parse_int_to_str(char *str, long long res, int *len, flg *f);
char *s21_parse_unsigned(char *str, va_list va, flg f);
char *s21_parse_float(char *str, va_list va, flg f);
void hex_prepare(flg f, int *letter, int *flg, int *numlength,
                 long long unsigned *hexnum, int *flg1);
int s21_check_double(long double argument, char **str, flg f);
char *s21_conc(char *str1, char *str2, flg f, int n);
char *s21_parse_float_to_str(char *str, long double res, flg f, int signFlag);
char *s21_print_precision(flg f, char *str_old, int len, int *count);
char *s21_print_width(flg f, char *str_old, int count, char *str);
int counter_numbers(long long unsigned int num);
void s21_parse_n(va_list va, int diff);
void change_flags(flg *f);
char *s21_parse_float(char *str, va_list va, flg f);
char *s21_parse_char(char *str, va_list va, flg f);
char *s21_parse_string(char *str, va_list va, flg f);
char *s21_parse_pointer(char *str, va_list va, flg f);
char *s21_parse_hexadecimal(char *str, va_list va, flg f);
char *s21_parse_octal(char *str, va_list va, flg f);
char *s21_parse_float_for_exponent(char *str, long double *res, flg f,
                                   int *grad, int secondLoop);
char *s21_parse_exponent(char *str, va_list va, flg f);
char *s21_print_width_exp(char *str, flg f, int widthCoeff, long long intPart,
                          int *widthFlg);
long double s21_change_exp_num(long double res, int *grad);
int s21_calc_precision(long double *res, flg *f);
char *s21_calc_width_octal(char *str, int numlength, flg f, int flg,
                           char tmp[]);
int s21_abs(int num);

#endif
