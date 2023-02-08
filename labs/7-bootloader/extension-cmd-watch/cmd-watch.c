// simple program to run a command when any file that is "interesting" in a directory
// changes.
// e.g., 
//      cmd-watch make
// will run make at each change.
//
// This should use the scandir similar to how you did `find_ttyusb`
//
// key part will be implementing two small helper functions (useful-examples/ will be 
// helpful):
//  - static int pid_clean_exit(int pid);
//  - static void run(char *argv[]);
//
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>

static const char* FILE_SUFFIXES[] = { ".c", ".h", ".S", "Makefile", 0 };
static time_t last_mtime;   // store last modification time.

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    const char** pointer = FILE_SUFFIXES;
    while (*pointer != NULL) {
        if (strstr(d->d_name, *pointer) != NULL) {
            char full_path[strlen(d->d_name) + 5];
            full_path[0] = 0;
            strcat(full_path, "./");
            strcat(full_path, d->d_name);
            struct stat info;
            int ret = stat(full_path, &info);
            if (ret == -1) {
                perror("stat failed");
            }
            else if (info.st_mtime > last_mtime) {
                last_mtime = info.st_mtime;
                printf("\nFound later file %s at time %ld\n", full_path, last_mtime);
                return 1;   
            }
            return 0;
        }
        pointer++;
    }
    return 0;
}


// scan the files in "./" (you can extend this) for those
// that match the suffixes in <suffixes> and check  if any
// have changed since the last time.
int check_activity(void) {
    const char *dirname = ".";

    struct dirent** files;
    int num_files = scandir(dirname, &files, filter, alphasort);
    if (num_files == -1) {
        panic("scandir failed");
    }
    for (int i = 0; i < num_files; i++) {
        free(files[i]);
    }
    free(files);
    // return 1 if anything that matched <suffixes>
    return num_files > 0;
}


// synchronously wait for <pid> to exit.  returns 1 if it exited
// cleanly (via exit(0)), 0 otherwise.
static int pid_clean_exit(int pid) {
    unimplemented();
}

// simple helper to print null terminated vector of strings.
static void print_argv(char *argv[]) {
    assert(argv[0]);

	fprintf(stderr, "about to exec = <%s", argv[0]);
	for(int i =1; argv[i]; i++)
		fprintf(stderr, " %s", argv[i]);
	fprintf(stderr, ">\n");
}


// fork/exec <argv> and wait for the result: print an error
// and exit if the kid crashed or exited with an error (a non-zero
// exit code).
static void run(char *argv[]) {
    int pipe_fd[2];
    int ret = pipe(pipe_fd);
    if (ret == -1) {
        panic("pipe failed");
    }
    pid_t pid = fork();
    if (pid == -1) {
        panic("fork failed");
    }

    if (pid == 0) {
        // child
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        int err = execvp(argv[0], argv);
        if (err == -1) {
            panic("Error: could not execute command %s", argv[0]);
        }
    }
    else {
        // parent
        close(pipe_fd[1]);
        int status;
        waitpid(pid, &status, 0);
        char output[4096];
        int num_read = read(pipe_fd[0], output, sizeof(output));
        output[num_read] = 0;
        close(pipe_fd[0]);
        printf("Read from child: %d bytes\n", num_read);
        printf("******************************************************\n");
        printf("%s", output);
        printf("******************************************************\n");
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit status: %d\n", exit_code);
        }
        else {
            panic("Error: program %s did not exit cleanly", argv[0]);
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2)
        die("cmd-watch: not enough arguments\n");
        
    char** program_args = argv + 1;
    print_argv(program_args);

    while (true) {
        if (check_activity()) {
            run(program_args);
        }
        usleep(500000);
    }

    return 0;
}
