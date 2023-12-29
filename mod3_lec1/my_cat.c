#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

enum ERRORS {
    OPEN_ERROR = 1,
    READ_ERROR,
    WRITE_ERROR
};

#define OPEN_ERROR_MSG_PREFIX_FORMAT_STR "Failed opening %s: \n"
#define READ_ERROR_MSG_PREFIX_FORMAT_STR "Failed reading from %s: \n"
#define WRITE_ERROR_MSG_PREFIX_FORMAT_STR "Failed writing data from %s: \n"

int open_file_for_reading(const char* path) {
    assert(NULL != path);
    return open(path, O_RDONLY);
}

int print_fd_to_stdout(int fd) {
    assert(0 <= fd);
    char buf[BUFSIZ];
    ssize_t read_result = 0;
    ssize_t write_result = 0;
    while ((read_result = read(fd, buf, BUFSIZ)) > 0) {
        if ((write_result = write(1, buf, read_result)) < 0) {
            return WRITE_ERROR;
        }
    }
    if (0 != read_result) {
        // Not EOF
        return READ_ERROR;
    }
    return 0;
}


/**
 * Prints concatenated contents of all arguments
 * In case of zero arguments reads from stdin, 
 *  this makes 'cat > out.txt' usage possible with this cat
*/
int main(int argc, char** argv) {
    int files_num = argc - 1;

    if (0 == files_num) {
        int print_result = print_fd_to_stdout(0);
        if (0 != print_result) {
            if (READ_ERROR == print_result) {
                fprintf(stderr, READ_ERROR_MSG_PREFIX_FORMAT_STR, "stdin");
                perror(NULL);
            }
            if (WRITE_ERROR == print_result) {
                fprintf(stderr, WRITE_ERROR_MSG_PREFIX_FORMAT_STR, "stdin");
                perror(NULL);
            }
            return print_result;
        }
        return 0;
    }

    for (int i = 0; i < files_num; ++i) {
        char* path = argv[i + 1];
        int fd = open_file_for_reading(path);
        errno = 0;
        if (0 > fd) {
            fprintf(stderr, OPEN_ERROR_MSG_PREFIX_FORMAT_STR, path);
            perror(NULL);
            return OPEN_ERROR;
        }
        int print_result = print_fd_to_stdout(fd);
        if (0 != print_result) {
            if (READ_ERROR == print_result) {
                fprintf(stderr, READ_ERROR_MSG_PREFIX_FORMAT_STR, path);
                perror(NULL);
            }
            if (WRITE_ERROR == print_result) {
                fprintf(stderr, WRITE_ERROR_MSG_PREFIX_FORMAT_STR, path);
                perror(NULL);
            }
            close(fd);
            return print_result;
        }
        close(fd);
    }

    return 0;
}
