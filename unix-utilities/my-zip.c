#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
// At least one file is required
    if (argc == 1) {
        printf("my-zip: file1 [file2 ...]\n");
        return 1;
    }

    int count = 0;
    int current = EOF;
// Threat all input files as one continuous stream
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");

        if (fp == NULL) {
            printf("my-zip: cannot open file\n");
            return 1;
        }

        int c;

        while ((c = fgetc(fp)) != EOF) {
	    // First run
            if (current == EOF) {
                current = c;
                count = 1;
	    // Count a current character
            } else if (c == current) {
                count++;
	    // Character changed and write accomplsihed run
            } else {
                fwrite(&count, sizeof(int), 1, stdout);
                fputc(current, stdout);

                current = c;
                count = 1;
            }
        }

        fclose(fp);
    }
// Write a final run after all files are processed
    if (current != EOF) {
        fwrite(&count, sizeof(int), 1, stdout);
        fputc(current, stdout);
    }

    return 0;
}
