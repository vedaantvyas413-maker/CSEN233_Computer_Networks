#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

void copy_file_sys(const char *src, const char *dest){
    clock_t start, end;
    double cpu_time_used;
    
    int fd = open(src, O_RDONLY);
    if (fd == -1) {
        perror("Error opening source file");
        return;
    }

    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(fd);
        return;
    }

    start = clock();
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(dest_fd, buffer, bytes_read);
    }
    end = clock();

    close(fd);
    close(dest_fd);
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("File copy completed in %f seconds.\n", cpu_time_used);
}


int main (int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <arg1> <arg2>\n", argv[0]);
        return 1;
    }
    printf("Copying Files using System Calls\n");
    copy_file_sys(argv[1], argv[2]);
    return 0;
}
