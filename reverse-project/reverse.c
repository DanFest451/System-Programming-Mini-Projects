#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//  Free every allocated input line and  a pointer array
void free_lines(char **lines, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        free(lines[i]);
    }

    free(lines);
}

int main(int argc, char *argv[])
{
// Program accepts both an input file and an output file
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
/* Cheching filenames of input and output are the same before 
opening an output file */
    if (argc == 3 && strcmp(argv[1], argv[2]) == 0) {
         fprintf(stderr, "Input and output file must differ\n");
         exit(1);
    }
// Streams are replased with files when filenames are submitted
    FILE *input = stdin;
    FILE *output = stdout;
// Open an input file 
    if (argc >= 2) {
        input = fopen(argv[1], "r");

        if (input == NULL) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }
/* Compare a device and inode numbers to find out weither 
different pathnames can still refer to the same physical file*/
   if (argc == 3) {
        struct stat input_stat;
        struct stat output_stat;

        if (stat(argv[1], &input_stat) == 0 &&
            stat(argv[2], &output_stat) == 0) {

            if (input_stat.st_dev == output_stat.st_dev &&
                input_stat.st_ino == output_stat.st_ino) {

                fprintf(stderr, "Input and output file must differ\n");
                fclose(input);
                exit(1);
            }
        }
    }
// Open an output file
    if (argc == 3) {
  	output = fopen(argv[2], "w");

        if (output == NULL) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);

            if (input != stdin) {
            	 fclose(input);
            }

            exit(1);
       	}
    }

    char *line = NULL;
    size_t size = 0;
// Store pointer to the line. Number of lines is unknown
    char **lines = NULL;
    size_t count = 0;
    size_t capacity = 0;

    while (getline(&line, &size, input) != -1) {
	// Increase capacity of pointer arrays if it's full
        if (count == capacity) {
            size_t new_capacity;

            if (capacity == 0) {
                new_capacity = 10;
            } else {
                new_capacity = capacity * 2;
            }

            char **temp = realloc(lines, new_capacity * sizeof(char *));

            if (temp == NULL) {
 	         fprintf(stderr, "malloc failed\n");

                 free_lines(lines, count);
                 free(line);

                 if (input != stdin) {
                     fclose(input);
                 }

                 if (output != stdout) {
                     fclose(output);
                 }

                 exit(1);
            }

            lines = temp;
            capacity = new_capacity;
        }
	// Each input line needs its own allocation because of getline()
        lines[count] = malloc(strlen(line) + 1);

        if (lines[count] == NULL) {
            fprintf(stderr, "malloc failed\n");

            free_lines(lines, count);
            free(line);

            if (input != stdin) {
                fclose(input);
            }

            if (output != stdout) {
                fclose(output);
            }

            exit(1);
        }

        strcpy(lines[count], line);
        count++;
    }
// Print stored lines from the last line to the first
    for (size_t i = count; i > 0; i--) {
    	fprintf(output, "%s", lines[i - 1]);
    }
// Release all allocated memory
    free_lines(lines, count);
    free(line);
// Close files that were opened by a program
    if (input != stdin) {
        fclose(input);
    }

    if (output != stdout) {
	fclose(output);
    }

    return 0;
}
