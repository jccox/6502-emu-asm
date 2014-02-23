/**********************************************************
 * Tiny bug: calling exit means garbage collection fails()
 * Other note: how useful is garbage collection at exit anyways
 * Think of a way to collect garbage based on scope
 * (is that even posible in c)
 *
 * Bug in manatorieness code (making everything manatory)
 *
 * A real parse tree might be helpful here
 *
 **********************************************************/
#include "optparse.h"

#define p(x)  ((x)-0x61)

#define set     \
{&fmt_error,    /*a*/     \
 &fmt_error,    /*b*/     \
 &c_assign,     /*c*/     \
 &d_assign,     /*d*/     \
 &fmt_error,    /*e*/     \
 &fmt_error,    /*f*/     \
 &fmt_error,    /*g*/     \
 &fmt_error,    /*h*/     \
 &fmt_error,    /*i*/     \
 &fmt_error,    /*j*/     \
 &fmt_error,    /*k*/     \
 &fmt_error,    /*l*/     \
 &fmt_error,    /*m*/     \
 &fmt_error,    /*n*/     \
 &fmt_error,    /*o*/     \
 &fmt_error,    /*p*/     \
 &fmt_error,    /*q*/     \
 &fmt_error,    /*r*/     \
 s_assign,      /*s*/     \
 &fmt_error,    /*t*/     \
 &fmt_error,    /*u*/     \
 &fmt_error,    /*v*/     \
 &fmt_error,    /*w*/     \
 &fmt_error,    /*x*/     \
 &fmt_error,    /*y*/     \
 &fmt_error,    /*z*/     \
}

void fmt_error(char *s, void *d);
void c_assign(char *s, void *d);
void d_assign(char *s, void *d);
void s_assign(char *s, void *d);

static void *fmt_lu[26] = set;

static const char sink = '\0';

static const char *usage_str;

void set_usage(const char *usage_msg)
{
	usage_str = usage_msg;
}

void call_usage()
{
	if(usage_str)
		fprintf(stderr, "%s\n", usage_str);
}

struct final_t{
	char used : 1;
	char manatory : 1;
	char *optstr;
	void *data;
	char type;
// 	char *modifiers;	//not implemented yet(if i really want these writing a full fledged parse tree may be the way to go)
};

void parse(int argc, char *argv[], struct final_t *final, int count);

void optparse(int argc, char *argv[], char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char *it = fmt, *prev = fmt;
	int qual = 0, idx = 0;

	struct {
		char *token;
		char option : 1;		//0 if option, 1 if fmt part
		unsigned char length : 7;	//assumes length of long option is less than 127
	} toks[(strlen(fmt)+1)/2];

	while(*it){
		if( (!qual&&(*it == '%')) || (qual&&(*it == ',')) || !(*(it + 1))){
			toks[idx].length = it - prev + (!(*(it + 1)));
			toks[idx].token = prev;
			toks[idx].option = qual;

			prev = it + 1;
			qual = !qual;
			idx++;
		}
		it++;
	}

	if(qual){
		fprintf(stderr, "runtime error: optparse: format string not balanced\n");
		exit(EXIT_FAILURE);
	}

	int count = (idx + 1)/2;
	idx = 0;

	struct final_t final[count];

	while(idx < count){
		final[idx].used = 0;

		if(toks[idx*2 + 1].token[0] == 'm')
			final[idx].manatory = 1;
		else
			final[idx].manatory = 0;

		if(idx >= (argc - 1)/2)
		{
			call_usage();
			exit(EXIT_FAILURE);
		}
		final[idx].data = va_arg(args, void *);
		final[idx].optstr = malloc(toks[idx*2].length + 3);		//two for spaces, two for dashes, and one for null
		if(!final[idx].optstr){
			perror("malloc");
			exit(ENOMEM);
		}

		if(toks[idx*2].length == 1){
			final[idx].optstr[0] = '-';
			final[idx].optstr[1] = toks[idx*2].token[0];
			final[idx].optstr[2] = '\0';
		}else{
			final[idx].optstr[0] = '-';
			final[idx].optstr[1] = '-';
			memcpy(final[idx].optstr + 2, toks[idx*2].token, toks[idx*2].length);
			final[idx].optstr[toks[idx*2].length + 2] = '\0';
		}

		final[idx].type = toks[idx*2 + 1].token[toks[idx*2 + 1].length - 1];
		//strings are special, we dont want to segfault on derefrance if the string is not provided
		if(final[idx].type == 's')
			*((char **)final[idx].data) = (char *)&sink;

		idx++;
	}

	parse(argc, argv, final, count);

	while(idx){
		idx--;
		free(final[idx].optstr);
	}

	va_end(args);
}

void parse(int argc, char *argv[], struct final_t *final, int count)
{
	for(int i = 0; i < argc; i++){
		for(int j = 0; j < count; j++){
			if(!strcmp(argv[i], final[j].optstr)){
				((void(*)(char *, void *))(fmt_lu[p(final[j].type)]))(argv[i + 1], final[j].data);
				final[j].used = 1;
				i++;
				break;
			}
		}
	}

	//check mandatorness has been fuffiled
	for(int i = 0; i < count; i++){
		if((final[i].manatory) && (!final[i].used)){
			call_usage();
			exit(EXIT_FAILURE);
		}
	}
}

void fmt_error(char *s, void *d)
{
	fprintf(stderr, "runtime error: optparse: invalid format string type\n");
	exit(EXIT_FAILURE);
}

void c_assign(char *s, void *d)
{
	*(char *)d = *s;
}

void d_assign(char *s, void *d)
{
	*(int *)d = atoi(s);
}

void s_assign(char *s, void *d)
{
	int len = strlen(s);
	char *str = malloc(len+1);
	if(!str){
		perror("malloc");
		exit(ENOMEM);
	}
	strcpy(str, s);

	auto_free_add(str);

	*(char **)d = str;
}
