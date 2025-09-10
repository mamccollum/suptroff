#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>
#include <termios.h>

/* trtime -- simple troff strftime preprocessor
 * USAGE: trtime [-] [file ...]
 * stdin or file, trtime takes lines beginning with ``.FTIME'' and
 * converts their arguments via strftime. If there are no arguments,
 * the locale's standard date+time is printed.
 * Molly A. McCollum <mam@sdf.org>
 */

void process(FILE *input, struct tm *tm_info) {
	/* char *ptr = NULL;
	size_t cap = 0; */

	char line[512];
	char timestr[512];
	char fmttimestr[512];
	regex_t re;
	regex_t re2;
	regmatch_t pmatch[1];
	regcomp(&re,  "^\\.\\s*FTIME" , REG_EXTENDED);
	regcomp(&re2, "^\\.\\s*FTIME\\s+", REG_EXTENDED);
	while(fgets(line, sizeof(line), input)) {
		if (regexec(&re, line, 0, NULL, 0) != 0) {
			printf("%s", line);
			continue;
		}

		bool has_real_args = false;
		if (regexec(&re2, line, 1, pmatch, 0) == 0) {
			char *p = line + pmatch[0].rm_eo;
			char *q = p;
			while (*q && isspace((unsigned char)*q)) q++;

			if (*q != '\0') {
				has_real_args = true;
				strcpy(timestr, p);
				strftime(fmttimestr, 512, timestr, tm_info);
				printf("%s", fmttimestr);
			}
		}
		if (!has_real_args) {
			char default_timestr[] = "%c\n";
			strftime(fmttimestr, 512, default_timestr, tm_info);
			printf("%s", fmttimestr);
		}
	}
	regfree(&re);
	regfree(&re2);
}

int main(int argc, char *argv[]) {
	FILE *input;

	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);

	bool stdin_defined = false;
	if(argc == 1) {
		input = stdin;
		stdin_defined = true;		
		process(input, tm_info);
	} else {
		for(int i=1; i < argc; i++) {
			if(argv[i][0] == '-' && argv[i][1] == '\0') {
				if(stdin_defined == true) {
					fprintf(stderr, "error: don't define stdin twice");
					exit(1); }
				input = stdin;
				stdin_defined = true;
			} else {
				input = fopen(argv[i], "r");
				if(input == NULL) {
					fprintf(stderr, "error: file %s does not exist", argv[i]);
					exit(2); }
			}
			process(input, tm_info);
			if(!stdin_defined) {
				fclose(input); }
		}
	}
	return 0;
}
