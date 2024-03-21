#include "s21_sprintf.h"

int s21_sprintf(char *str, const char *format, ...) {
  flg f = {0};
  const char *form = format;
  char *s = str;
  va_list arg;
  va_start(arg, format);
  while (*form) {
    if (*form != '%')
      *(s++) = *(form++);
    else {
      form++;
      s21_memset(&f, 0, N_FLAGS);
      get_flags(&form, &f);
      get_width(&form, &f, arg);
      get_precision(&form, &f, arg);
      get_length(&form, &f);
      f.specifier = *form;
      change_flags(&f);
      form++;
      s = s21_args_value(s, f, arg, str);
    }
  }
  *s = '\0';
  va_end(arg);
  return s - str;
}

char *s21_args_value(char *str, flg f, va_list va, char *str_start) {
  if (f.specifier == 'd' || f.specifier == 'i')
    str = s21_parse_int(str, va, f);
  else if (f.specifier == 'u')
    str = s21_parse_unsigned(str, va, f);
  else if (f.specifier == '%')
    *(str++) = '%';
  else if (f.specifier == 'n')
    s21_parse_n(va, str - str_start);
  else if (f.specifier == 'f')
    str = s21_parse_float(str, va, f);
  else if (f.specifier == 'c')
    str = s21_parse_char(str, va, f);
  else if (f.specifier == 's')
    str = s21_parse_string(str, va, f);
  else if (f.specifier == 'p')
    str = s21_parse_pointer(str, va, f);
  else if (f.specifier == 'x' || f.specifier == 'X')
    str = s21_parse_hexadecimal(str, va, f);
  else if (f.specifier == 'o')
    str = s21_parse_octal(str, va, f);
  else if (f.specifier == 'e' || f.specifier == 'E')
    str = s21_parse_exponent(str, va, f);
  return str;
}

char *s21_parse_exponent(char *str, va_list va, flg f) {
  long double res;
  if (f.length == 'L')
    res = va_arg(va, long double);
  else
    res = va_arg(va, double);
  int signFlag = s21_calc_precision(&res, &f);
  int grad = 0;

  res = s21_change_exp_num(res, &grad);
  s21_parse_float_for_exponent(str, &res, f, &grad, 1);
  res = s21_change_exp_num(res, &grad);

  long double tmp;
  modfl(res, &tmp);
  long long unsigned int intPart = (long long unsigned int)tmp;
  int widthCoeff = 0, widthFlg = 1;
  if (signFlag || f.plus || f.space) widthCoeff = 1;
  if (signFlag == 1 && f.zero)
    *(str++) = '-';
  else if (f.plus && f.zero)
    *(str++) = '+';
  else if (f.space && f.zero)
    *(str++) = ' ';
  str = s21_print_width_exp(str, f, widthCoeff, intPart, &widthFlg);
  if (signFlag == 1 && !f.zero)
    *(str++) = '-';
  else if (f.plus && !f.zero)
    *(str++) = '+';
  else if (f.space && !f.zero)
    *(str++) = ' ';
  str = s21_parse_float_for_exponent(str, &res, f, &grad, 0);
  *(str++) = f.specifier;
  if (grad < 0)
    *(str++) = '-';
  else
    *(str++) = '+';
  if (s21_abs(grad) < 10) *(str++) = '0';
  int len = 0;
  str = s21_parse_int_to_str(str, grad, &len, &f);
  str = s21_print_width_exp(str, f, widthCoeff, intPart, &widthFlg);
  return str;
}

int s21_calc_precision(long double *res, flg *f) {
  int signFlag = 0;
  if (*res < 0) {
    *res *= -1;
    signFlag = 1;
  }
  if (!f->precision && !f->is_precision) {
    f->precision = 6;
    f->is_precision = 1;
  }
  if (!f->precision && !f->hashtag) f->is_precision = 0;
  return signFlag;
}

long double s21_change_exp_num(long double res, int *grad) {
  if (res == 0) {
    *grad = 0;
  } else {
    while (res < 1) {
      (*grad)--;
      res *= 10;
    }
    while (res >= 10) {
      (*grad)++;
      res /= 10.0;
    }
  }
  return res;
}

char *s21_print_width_exp(char *str, flg f, int widthCoeff, long long intPart,
                          int *widthFlg) {
  char ch = ' ';
  if (f.zero) ch = '0';
  for (int j = 0; *widthFlg && !f.minus &&
                  j < f.width - (4 + counter_numbers(intPart) + f.precision +
                                 widthCoeff + f.is_precision);
       j++) {
    *(str++) = ch;
  }
  for (int j = 0; !*widthFlg && f.minus &&
                  j < f.width - (4 + counter_numbers(intPart) + f.precision +
                                 widthCoeff + f.is_precision);
       j++) {
    *(str++) = ' ';
  }
  *widthFlg = 0;
  return str;
}
int s21_abs(int num) {
  if (num < 0) num *= -1;
  return num;
}

char *s21_parse_float_for_exponent(char *str, long double *res, flg f,
                                   int *grad, int secondLoop) {
  static int lastNum;
  long long int tmpres = (long long int)*res;
  if (secondLoop) {
    long double tmp;
    long double floatPart = modfl((*res * pow(10, f.precision)), &tmp);
    lastNum = (long long unsigned int)(*res * pow(10, f.precision)) % 10;
    if (floatPart == 0.5 && lastNum % 2 != 0) {
      *res = *res * pow(10, f.precision) + 1;
    } else if (floatPart > 0.5) {
      *res = *res * pow(10, f.precision) + 1;
    } else {
      *res = *res * pow(10, f.precision);
    }
    *res /= pow(10, f.precision);
    f.firstLoop = 1;
  }
  long double intParts;
  long long int nineNum =
      (long long int)pow(10, f.precision) * modfl(*res, &intParts);
  if ((tmpres != intParts && tmpres > pow(10, -6) &&
       (long long)10 * modfl(*res, &intParts) > 5 && lastNum != 4 &&
       intParts == 9 && f.precision == 1) ||
      ((long long int)res == 9 && pow(10, f.precision) - nineNum == 1 &&
       f.precision != 0)) {
    *res = 10.0;
    (void)*grad++;
  }
  int signFlag = 0;
  str = s21_parse_float_to_str(str, *res, f, signFlag);
  f.firstLoop = 0;
  return str;
}

void s21_parse_n(va_list va, int diff) {
  int *res = va_arg(va, int *);
  *res = diff;
}

char *s21_parse_octal(char *str, va_list va, flg f) {
  long long unsigned hexnum;
  if (f.length == 'l')
    hexnum = va_arg(va, long unsigned);
  else if (f.length == 'h')
    hexnum = (short unsigned)va_arg(va, unsigned);
  else
    hexnum = va_arg(va, unsigned);
  char tmp[BUFF_SIZE];
  int numlength = 0, flg = 1, flg1 = 0;
  if (f.is_precision == 1 && f.precision == 0 && hexnum == 0 &&
      f.hashtag == 0) {
    flg = 0;
    numlength--;
  }
  if (hexnum == 0) {
    *tmp = '0';
    numlength++;
  }
  if (hexnum != 0 && f.hashtag) flg1++;
  for (; hexnum != 0; numlength++) {
    tmp[numlength] = (hexnum % 8) + 48;
    hexnum /= 8;
  }
  if (flg1) {
    tmp[numlength] = '0';
    numlength++;
  }
  str = s21_calc_width_octal(str, numlength, f, flg, tmp);
  return str;
}

char *s21_calc_width_octal(char *str, int numlength, flg f, int flg,
                           char tmp[]) {
  if (numlength > f.precision)
    for (int j = 0; !f.minus && f.width > (numlength + j); j++) *(str++) = ' ';
  else
    for (int j = 0; !f.minus && f.width > (f.precision + j); j++)
      *(str++) = ' ';
  for (int j = 0; f.precision > (numlength + j); j++) *(str++) = '0';
  for (int j = numlength; j > 0 && flg;) *(str++) = tmp[--j];
  if (numlength > f.precision)
    for (int j = 0; f.minus && f.width > (numlength + j); j++) *(str++) = ' ';
  else
    for (int j = 0; f.minus && f.width > (f.precision + j); j++) *(str++) = ' ';
  return str;
}

void hex_prepare(flg f, int *letter, int *flg, int *numlength,
                 long long unsigned *hexnum, int *flg1) {
  if (f.specifier == 'x') *letter = 87;
  if (f.is_precision == 1 && f.precision == 0 && *hexnum == 0) {
    *flg = 0;
    (*numlength)--;
  }
  if (*hexnum != 0 && f.hashtag) *flg1 += 2;
}

char *s21_parse_hexadecimal(char *str, va_list va, flg f) {
  long long unsigned hexnum;
  if (f.length == 'l')
    hexnum = va_arg(va, long unsigned);
  else if (f.length == 'h')
    hexnum = (short unsigned)va_arg(va, unsigned int);
  else
    hexnum = va_arg(va, unsigned);
  char tmp[BUFF_SIZE];
  int numlength = 0, letter = 55, flg1 = 0, flg = 1;
  hex_prepare(f, &letter, &flg, &numlength, &hexnum, &flg1);
  if (hexnum == 0) {
    *tmp = '0';
    numlength++;
  }
  for (; hexnum != 0; numlength++) {
    if (hexnum % 16 < 10)
      tmp[numlength] = (hexnum % 16) + 48;
    else
      tmp[numlength] = (hexnum % 16) + letter;
    hexnum /= 16;
  }
  if (numlength > f.precision)
    for (int j = flg1; !f.minus && f.width > (numlength + j); j++)
      *(str++) = ' ';
  else
    for (int j = flg1; !f.minus && f.width > (f.precision + j); j++)
      *(str++) = ' ';
  if (flg1) {  // New
    *(str++) = '0';
    *(str++) = f.specifier;
  }
  for (int j = 0; f.precision > (numlength + j); j++) *(str++) = '0';
  for (int j = numlength; j > 0 && flg;) *(str++) = tmp[--j];
  if (numlength > f.precision)
    for (int j = flg1; f.minus && f.width > (numlength + j); j++)
      *(str++) = ' ';
  else
    for (int j = flg1; f.minus && f.width > (f.precision + j); j++)
      *(str++) = ' ';
  return str;
}

char *s21_parse_pointer(char *str, va_list va, flg f) {
  int i = 0, n;
  int *ptr = va_arg(va, int *);
  long long unsigned hexnum = (long long unsigned)ptr;
  char tmp[BUFF_SIZE];
  for (; hexnum != 0; i++) {
    if (hexnum % 16 < 10) {
      tmp[i] = (hexnum % 16) + 48;
    } else {
      tmp[i] = (hexnum % 16) + 87;
    }
    hexnum /= 16;
  }
  n = i;
  for (int j = 0; !f.minus && j < f.width - n - 2; j++) *(str++) = ' ';
  *(str++) = '0';
  *(str++) = 'x';
  for (; i > 0;) *(str++) = tmp[--i];
  for (int j = 0; f.minus && j < f.width - n - 2; j++) *(str++) = ' ';
  return str;
}

char *s21_parse_string(char *str, va_list va, flg f) {
  char *res = va_arg(va, char *);
  int lenStr = 0;
  while (*(res++) != '\0') lenStr++;
  res -= (lenStr + 1);
  if (f.is_precision && f.precision == 0)
    lenStr = 0;
  else if (f.is_precision && f.precision != 0)
    lenStr = f.precision;
  for (int i = 0; !f.minus && i < f.width - lenStr; i++) *(str++) = ' ';
  for (int i = 0; *res != '\0' && i < lenStr; i++) *(str++) = *(res++);
  for (int i = 0; f.minus && i < f.width - lenStr; i++) *(str++) = ' ';
  return str;
}

char *s21_parse_char(char *str, va_list va, flg f) {
  char res = (char)va_arg(va, int);
  for (int i = 0; !f.minus && i < f.width - 1; i++) {
    *(str++) = ' ';
  }
  *(str++) = res;
  for (int i = 0; f.minus && i < f.width - 1; i++) {
    *(str++) = ' ';
  }
  return str;
}

char *s21_parse_int(char *str, va_list va, flg f) {
  long long res;
  if (f.length == 'h') {
    res = va_arg(va, int);
    res = (short int)res;
  } else if (f.length == 0)
    res = va_arg(va, int);
  else if (f.length == 'l')
    res = va_arg(va, long int);
  char *str_old = str;
  int len = 0, count = 0;
  str = s21_parse_int_to_str(str, res, &len, &f);
  str = s21_print_precision(f, str_old, len, &count);
  str = s21_print_width(f, str_old, count, str);
  return str;
}

char *s21_print_width(flg f, char *str_old, int count, char *str) {
  int c = 0;
  char ch = ' ';
  if (f.zero) ch = '0';
  if (f.width > count) {
    c = f.width - count;
    if (f.minus == 0) {
      char tmp[BUFF_SIZE] = {'\0'};
      char *str_o = str_old;
      for (int i = 0; i < count; i++) tmp[i] = *(str_o++);
      for (int i = 0; i < f.width - count; i++) *(str_old++) = ch;
      for (int i = 0; i < count; i++) *(str_old++) = tmp[i];
    } else {
      char *s = str;
      for (int i = 0; i < f.width - count; i++) *(s++) = ch;
    }
  }
  return str + c;
}

char *s21_print_precision(flg f, char *str_old, int len, int *count) {
  char tmp[BUFF_SIZE] = {'\0'};
  int sign = 0;
  if (*str_old == '-' || *str_old == '+' || *str_old == ' ') {
    tmp[0] = *str_old;
    (*count)++;
    sign = 1;
    len--;
  }
  if (f.precision > len) {
    int i;
    for (i = sign; i < f.precision - len + sign; i++) {
      tmp[i] = '0';
      (*count)++;
    }
    for (int j = sign; j < len + sign; j++, i++) {
      tmp[i] = str_old[j];
      (*count)++;
    }
    s21_strncpy(str_old, tmp, s21_strlen(tmp));
  } else
    for (int j = sign; j < len + sign; j++) (*count)++;
  if (f.is_precision && !f.precision &&
      (f.specifier == 'd' || f.specifier == 'u')) {
    if ((str_old[0] == '0' || (str_old[0] == '-' && str_old[1] == '0')))
      (*count) = 0;
    else if (((str_old[0] == '+' && str_old[1] == '0') ||
              (str_old[0] == ' ' && str_old[1] == '0')))
      (*count) = 1;
  }
  return (*count) + str_old;
}

char *s21_parse_int_to_str(char *str, long long res, int *len, flg *f) {
  if (f->plus && f->space && f->specifier == 'd') f->space = 0;
  char tmp[BUFF_SIZE] = {'\0'};
  int i = BUFF_SIZE - 2;
  int neg = res < 0 ? 1 : 0;
  res = neg ? -res : res;
  if (!neg && f->space && f->specifier == 'd') {
    *(str++) = ' ';
    (*len)++;
  }
  if (!res) {  // Если число равно нулю
    if (f->plus && f->specifier == 'd') {
      *(str++) = '+';
      (*len)++;
    }
    *(str++) = '0';
    (*len)++;
  }
  while (res > 0) {
    tmp[--i] = res % 10 + 48;
    res /= 10;
  }
  for (int j = 0; tmp[i] != '\0'; i++, j++, str++) {
    if (!j && neg && f->specifier == 'd') {
      *(str++) = '-';
      (*len)++;
    } else if (!j && f->plus && f->specifier == 'd') {
      f->space = 0;
      *(str++) = '+';
      (*len)++;
    }
    *str = tmp[i];
    (*len)++;
  }
  return str;
}

char *s21_parse_unsigned(char *str, va_list va, flg f) {
  unsigned long long res;  // Успешно
  if (f.length == 'h') {
    res = va_arg(va, unsigned int);
    res = (short unsigned int)res;
  } else if (f.length == 0)
    res = va_arg(va, unsigned int);
  else if (f.length == 'l')
    res = va_arg(va, unsigned long);
  char *str_old = str;
  int len = 0, count = 0;
  str = s21_parse_int_to_str(str, res, &len, &f);
  str = s21_print_precision(f, str_old, len, &count);
  str = s21_print_width(f, str_old, count, str);
  return str;
}

char *s21_parse_float(char *str, va_list va, flg f) {
  long double res;
  if (f.length == 'L')
    res = va_arg(va, long double);
  else
    res = va_arg(va, double);

  int signFlag = 0;
  if (res < 0) {
    signFlag = 1;
    res *= -1;
  }

  if (!f.precision && !f.is_precision) {
    f.precision = 6;
    f.is_precision = 1;
  }
  if (!f.precision && !f.hashtag) f.is_precision = 0;

  long double tmp;
  double floatPart = modfl((res * pow(10, f.precision)), &tmp);
  int lastNum = (long long unsigned int)(res * pow(10, f.precision)) % 10;
  if (floatPart == 0.5 && lastNum % 2 != 0) {
    res = res * pow(10, f.precision) + 1;
  } else if (floatPart > 0.5) {
    res = res * pow(10, f.precision) + 1;
  } else {
    res = res * pow(10, f.precision);
  }
  res /= pow(10, f.precision);
  str = s21_parse_float_to_str(str, res, f, signFlag);
  return str;
}

char *s21_parse_float_to_str(char *str, long double res, flg f, int signFlag) {
  long double tmp;
  long double floatPart;
  if (f.precision < 18 && f.firstLoop) res += pow(10, -18);
  floatPart = modfl(res, &tmp);
  char ch = ' ';
  if (f.zero) ch = '0';
  long long unsigned int intPart = (long long unsigned int)tmp;
  int len = 0;
  int widthCoeff = 0;
  if (signFlag || f.plus || f.space) widthCoeff = 1;
  if (signFlag == 1 && f.specifier == 'f' && f.zero)
    *(str++) = '-';
  else if (f.plus && f.specifier == 'f' && f.zero)
    *(str++) = '+';
  else if (f.space && f.specifier == 'f' && f.zero)
    *(str++) = ' ';
  for (int i = 0; !f.minus && f.specifier == 'f' &&
                  i < f.width - (counter_numbers(intPart) + f.precision +
                                 widthCoeff + f.is_precision);
       i++) {
    *(str++) = ch;
  }
  if (signFlag == 1 && f.specifier == 'f' && f.zero == 0)
    *(str++) = '-';
  else if (f.plus && f.specifier == 'f' && f.zero == 0)
    *(str++) = '+';
  else if (f.space && f.specifier == 'f' && f.zero == 0)
    *(str++) = ' ';
  str = s21_parse_int_to_str(str, intPart, &len, &f);
  if (f.precision != 0 || f.hashtag) *(str++) = '.';
  for (int i = 0; i < f.precision; i++, str++) {
    long long int rounded = floatPart * 10;
    *str = (char)(rounded % 10 + 48);
    floatPart *= 10;
    floatPart -= (long long int)floatPart;
  }
  for (int i = 0; f.minus && f.specifier == 'f' &&
                  i < f.width - (counter_numbers(intPart) + f.precision +
                                 widthCoeff + f.is_precision);
       i++)
    *(str++) = ' ';
  return str;
}

int counter_numbers(long long unsigned int num) {
  int i = 0, flag = 1;
  if (num == 0) {
    flag = 0;
    i = 1;
  }
  while (num / 1 != 0 && flag) {
    num /= 10;
    i++;
  }
  return i;
}

int s21_parse_flg(char *tmp) {
  int res = 0;
  for (int i = 0; tmp[i]; i++) res = 10 * res + (int)tmp[i] - 48;
  return res;
}

void get_flags(const char **form, flg *f) {
  while (**form == '-' || **form == '+' || **form == ' ' || **form == '0' ||
         **form == '#') {
    if (**form == '+') f->plus = 1;
    if (**form == '-') f->minus = 1;
    if (**form == ' ') f->space = 1;
    if (**form == '0') f->zero = 1;
    if (**form == '#') f->hashtag = 1;
    (*form)++;
  }
}

void get_width(const char **form, flg *f, va_list va) {
  if (s21_isdigit(**form)) {
    char tmp[BUFF_SIZE] = {'\0'};
    for (int i = 0; s21_isdigit(**form); (*form)++, i++) tmp[i] = **form;
    f->width = s21_parse_flg(tmp);
  } else if (**form == '*') {
    (*form)++;
    f->width = va_arg(va, int);
  }
}

void get_precision(const char **form, flg *f, va_list va) {
  if (**form == '.') {
    f->is_precision = 1;
    (*form)++;
    if (**form == '*') {
      (*form)++;
      f->precision = va_arg(va, int);
    } else {
      char tmp[BUFF_SIZE] = {'\0'};
      for (int i = 0; s21_isdigit(**form); (*form)++, i++) tmp[i] = **form;
      f->precision = s21_parse_flg(tmp);
    }
  }
}

void get_length(const char **form, flg *f) {
  if (**form == 'h') {
    f->length = 'h';
    (*form)++;
  }
  if (**form == 'l') {
    f->length = 'l';
    (*form)++;
  }
  if (**form == 'L') {
    f->length = 'L';
    (*form)++;
  }
}

void change_flags(flg *f) {
  if (f->plus && f->space && f->specifier == 'd') f->space = 0;
  if (f->zero && f->minus) f->zero = 0;
  if (f->zero && f->is_precision &&
      (f->specifier == 'd' || f->specifier == 'i' || f->specifier == 'o' ||
       f->specifier == 'u' || f->specifier == 'x' || f->specifier == 'X'))
    f->zero = 0;
}

int s21_isdigit(char c) { return c >= '0' && c <= '9'; }
