#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
// Search term is required
    if (argc == 1) {
        printf("my-grep: searchterm [file ...]\n");
        return 1;
    }

    char *searchterm = argv[1];
// Read from standard input if files aren't given
    if (argc == 2) {
        char *line = NULL;
        size_t len = 0;

        while (getline(&line, &len, stdin) != -1) {
            if (strstr(line, searchterm) != NULL) {
                printf("%s", line);
            }
        }

        free(line);
        return 0;
    }
// Search on a command line
    for (int i = 2; i < argc; i++) {
         FILE *fp = fopen(argv[i], "r");

         if (fp == NULL) {
             printf("my-grep: cannot open file\n");
             return 1;
         }

         char *line = NULL;
         size_t len = 0;
	 // getline() enables to read lines of lentgh
         while (getline(&line, &len, fp) != -1) {
             if (strstr(line, searchterm) != NULL) {
                 printf("%s", line);
             }
         }

         free(line);
         fclose(fp);
    }

    return 0;
}
