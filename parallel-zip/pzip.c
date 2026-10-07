#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>

typedef struct {
    int count;
    char character;
} Run;

typedef struct {
    char *filename;
    Run *runs;
    int run_count;
    int capacity;
    int error;
} FileResult;

// Compress one file into an array of RLE runs
void *compress_file(void *arg) {
    FileResult *result = (FileResult *)arg;

    FILE *fp = fopen(result->filename, "r");

    if (fp == NULL) {
        result->error = 1;
        return NULL;
    }

    result->capacity = 16;
    result->run_count = 0;
    result->runs = malloc(result->capacity * sizeof(Run));

    if (result->runs == NULL) {
        fclose(fp);
        result->error = 1;
        return NULL;
    }

    int current = fgetc(fp);

    // Empty file
    if (current == EOF) {
        fclose(fp);
        return NULL;
    }

    int count = 1;
    int c;

    while ((c = fgetc(fp)) != EOF) {
        if (c == current) {
            count++;
        } else {

            // Increase the run array if it becomes full
            if (result->run_count == result->capacity) {
                result->capacity *= 2;

                Run *temp = realloc(
                    result->runs,
                    result->capacity * sizeof(Run)
                );

                if (temp == NULL) {
                    free(result->runs);
                    result->runs = NULL;
                    fclose(fp);
                    result->error = 1;
                    return NULL;
                }

                result->runs = temp;
            }

            // Store the completed run
            result->runs[result->run_count].count = count;
            result->runs[result->run_count].character = (char)current;
            result->run_count++;

            current = c;
            count = 1;
        }
    }

    // Store the final run
    if (result->run_count == result->capacity) {
        result->capacity *= 2;

        Run *temp = realloc(
            result->runs,
            result->capacity * sizeof(Run)
        );

        if (temp == NULL) {
            free(result->runs);
            result->runs = NULL;
            fclose(fp);
            result->error = 1;
            return NULL;
        }

        result->runs = temp;
    }

    result->runs[result->run_count].count = count;
    result->runs[result->run_count].character = (char)current;
    result->run_count++;

    fclose(fp);
    return NULL;
}

int main(int argc, char *argv[]) {

    // At least one input file is required
    if (argc < 2) {
        printf("pzip: file1 [file2 ...]\n");
        return 1;
    }

    int num_files = argc - 1;

    // Determine the number of available CPU cores
    int num_threads = get_nprocs();

    if (num_threads < 1) {
        num_threads = 1;
    }

    if (num_threads > num_files) {
        num_threads = num_files;
    }

    FileResult *results = calloc(num_files, sizeof(FileResult));
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    if (results == NULL || threads == NULL) {
        free(results);
        free(threads);
        return 1;
    }

    // Store the input filenames
    for (int i = 0; i < num_files; i++) {
        results[i].filename = argv[i + 1];
    }

    for (int start = 0; start < num_files; start += num_threads) {

        int batch_size = num_threads;

        if (start + batch_size > num_files) {
            batch_size = num_files - start;
        }

        // Start worker threads
        for (int i = 0; i < batch_size; i++) {
            int index = start + i;

            if (pthread_create(
                    &threads[i],
                    NULL,
                    compress_file,
                    &results[index]) != 0) {

                fprintf(stderr, "pzip: could not create thread\n");

                for (int j = 0; j < i; j++) {
                    pthread_join(threads[j], NULL);
                }

                for (int j = 0; j < num_files; j++) {
                    free(results[j].runs);
                }

                free(results);
                free(threads);
                return 1;
            }
        }

        for (int i = 0; i < batch_size; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    // Check whether any input file failed
    for (int i = 0; i < num_files; i++) {
        if (results[i].error) {
            fprintf(stderr, "pzip: cannot open file\n");

            for (int j = 0; j < num_files; j++) {
                free(results[j].runs);
            }

            free(results);
            free(threads);
            return 1;
        }
    }

    /*
     * Output the compressed runs in original file order.
     * Adjacent identical runs are merged, including across file boundaries.
     */
    int have_pending = 0;
    Run pending = {0, 0};

    for (int i = 0; i < num_files; i++) {
        for (int j = 0; j < results[i].run_count; j++) {
            Run current = results[i].runs[j];

            if (!have_pending) {
                pending = current;
                have_pending = 1;
            } else if (pending.character == current.character) {
                pending.count += current.count;
            } else {
                fwrite(&pending.count, sizeof(int), 1, stdout);
                fwrite(&pending.character, sizeof(char), 1, stdout);

                pending = current;
            }
        }
    }

    // Write the final run
    if (have_pending) {
        fwrite(&pending.count, sizeof(int), 1, stdout);
        fwrite(&pending.character, sizeof(char), 1, stdout);
    }

    for (int i = 0; i < num_files; i++) {
        free(results[i].runs);
    }

    free(results);
    free(threads);

    return 0;
}
