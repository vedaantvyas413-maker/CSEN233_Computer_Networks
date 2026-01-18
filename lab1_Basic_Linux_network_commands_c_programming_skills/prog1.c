#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void copy_file(const char *src, const char *dest)
{  
    clock_t start, end;
    double cpu_time_used;
    FILE *src_file;
    FILE *dest_file;

    src_file = fopen(src, "r");
    if (!src_file) {
        perror("Error opening source file");
        return;
    }

    dest_file = fopen(dest, "w");
    if (!dest_file) {
        perror("Error opening destination file");
        fclose(src_file);
        return;
    }
    start = clock();

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        fwrite(buffer, 1, bytes_read, dest_file);
    }
    end = clock();

    fclose(src_file);
    fclose(dest_file);

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("File copy completed in %f seconds.\n", cpu_time_used);
}

int main (int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <arg1> <arg2>\n", argv[0]);
        return 1;
    }
    printf("Copying Files using Standard I/O\n");
    copy_file(argv[1], argv[2]);
    return 0;
}