#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
// If files aren't given, then exit
    if (argc == 1) {
        return 0;
    }
// Process every file given on  a command line
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
	// If a file cannot be opened, then stop
        if (fp == NULL) {
            printf("my-cat: cannot open file\n");
            return 1;
        }

        char buffer[1024];
	// Read and write a file contents
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }

        fclose(fp);
    }

    return 0;
}
