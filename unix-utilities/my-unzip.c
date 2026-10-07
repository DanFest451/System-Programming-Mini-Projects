#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
// At least one compressed file is required
    if (argc == 1) {
        printf("my-unzip: file1 [file2 ...]\n");
        return 1;
    }
// Process a whole compressed file
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");

        if (fp == NULL) {
            printf("my-unzip: cannot open file\n");
            return 1;
        }

        int count;
        char c;
	/* Each compressed entry includes an integer 
	and one charachter */ 
        while (fread(&count, sizeof(int), 1, fp) == 1) {
            if (fread(&c, sizeof(char), 1, fp) != 1) {
                break;
            }
	    // Reconstruct run
            for (int j = 0; j < count; j++) {
                printf("%c", c);
            }
        }

        fclose(fp);
    }

    return 0;
}
