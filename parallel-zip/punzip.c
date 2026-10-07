#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>

typedef struct {
    char *filename;
    char *data;
    size_t size;
    size_t capacity;
    int error;
} FileResult;

// Decompress one compressed file into memory
void *decompress_file(void *arg) {
    FileResult *result = (FileResult *)arg;

    FILE *fp = fopen(result->filename, "r");

    if (fp == NULL) {
        result->error = 1;
        return NULL;
    }

    result->capacity = 1024;
    result->size = 0;
    result->data = malloc(result->capacity);

    if (result->data == NULL) {
        fclose(fp);
        result->error = 1;
        return NULL;
    }

    int count;
    char character;

    while (fread(&count, sizeof(int), 1, fp) == 1) {

        if (fread(&character, sizeof(char), 1, fp) != 1) {
            result->error = 1;
            break;
        }

        // Checking for space for the decompressed run
        while (result->size + (size_t)count > result->capacity) {
            result->capacity *= 2;

            char *temp = realloc(result->data, result->capacity);

            if (temp == NULL) {
                free(result->data);
                result->data = NULL;
                fclose(fp);
                result->error = 1;
                return NULL;
            }

            result->data = temp;
        }

        // Reconstruct run in memory
        for (int i = 0; i < count; i++) {
            result->data[result->size++] = character;
        }
    }

    fclose(fp);
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("punzip: file1 [file2 ...]\n");
        return 1;
    }

    int num_files = argc - 1;
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

    for (int i = 0; i < num_files; i++) {
        results[i].filename = argv[i + 1];
    }

    // Process compressed files in parallel
    for (int start = 0; start < num_files; start += num_threads) {

        int batch_size = num_threads;

        if (start + batch_size > num_files) {
            batch_size = num_files - start;
        }

        for (int i = 0; i < batch_size; i++) {
            int index = start + i;

            if (pthread_create(
                    &threads[i],
                    NULL,
                    decompress_file,
                    &results[index]) != 0) {

                fprintf(stderr, "punzip: could not create thread\n");

                for (int j = 0; j < i; j++) {
                    pthread_join(threads[j], NULL);
                }

                for (int j = 0; j < num_files; j++) {
                    free(results[j].data);
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

    // Check for errors before producing output
    for (int i = 0; i < num_files; i++) {
        if (results[i].error) {
            fprintf(stderr, "punzip: cannot open or read file\n");

            for (int j = 0; j < num_files; j++) {
                free(results[j].data);
            }

            free(results);
            free(threads);
            return 1;
        }
    }

    // Output decompressed data in original file order
    for (int i = 0; i < num_files; i++) {
        fwrite(results[i].data, 1, results[i].size, stdout);
    }

    for (int i = 0; i < num_files; i++) {
        free(results[i].data);
    }

    free(results);
    free(threads);

    return 0;
}
