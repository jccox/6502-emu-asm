#ifndef OPTPARSE_H_INCLUDED
#define OPTPARSE_H_INCLUDED

#include <stdarg.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

void set_usage(const char *usage_msg);
void optparse(int argc, char *argv[], char *fmt, ...);

#endif//OPTPARSE_H_INCLUDED
