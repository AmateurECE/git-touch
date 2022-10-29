///////////////////////////////////////////////////////////////////////////////
// NAME:            git-touch.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the git-touch application
//
// CREATED:         11/04/2021
//
// LAST EDITED:     10/29/2022
//
// Copyright 2021, Ethan D. Twardy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////

#include <argp.h>
#include <bsd/string.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char* argp_program_version = "git-touch 0.2.0";
const char* argp_program_bug_address = "<ethan.twardy@gmail.com>";

static char doc[] = "Create and track changes to files with a single command";
static char args_doc[] = "filename";
static const int NUMBER_OF_ARGUMENTS = 1;

static const char* GIT = "git";

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    char** filename = state->input;
    switch (key) {
    case ARGP_KEY_ARG:
        if (state->arg_num >= NUMBER_OF_ARGUMENTS) {
            argp_usage(state);
        }

        *filename = arg;
        break;
    case ARGP_KEY_END:
        if (state->arg_num < NUMBER_OF_ARGUMENTS) {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {0, parse_opt, args_doc, doc, 0, 0, 0};

// Check to see if the git executable is in this directory
int git_in_dir(const char* path) {
    struct dirent* entry = NULL;
    DIR* directory = opendir(path);
    char error_message[80];
    if (NULL == directory) {
        strerror_r(errno, error_message, sizeof(error_message));
        fprintf(stderr, "Cannot open directory %s: %s\n", path, error_message);
        return errno;
    }

    while (NULL != (entry = readdir(directory))) {
        if (0 == strcmp(entry->d_name, GIT)) {
            return 0;
        }
    }

    closedir(directory);
    return ENOENT;
}

// Find git in the environment
int find_git(char* path, size_t path_length) {
    char* env_path = getenv("PATH");
    char* dir = NULL;
    char* saveptr = NULL;

    for (dir = strtok_r(env_path, ":", &saveptr); dir;
         dir = strtok_r(NULL, ":", &saveptr)) {
        int result = git_in_dir(dir);
        if (0 == result) {
            size_t dir_length = strlen(dir);
            size_t git_length = strlen(GIT);
            if (path_length < dir_length + 1 + git_length + 1) {
                return ERANGE;
            }

            strlcpy(path, dir, path_length);
            path[dir_length] = '/';
            strlcpy(path + dir_length + 1, GIT, path_length - dir_length - 1);
            return 0;
        } else if (ENOENT == result) {
            continue; // Not found, but keep going
        } else {
            return result; // Error occurred
        }
    }

    return ENOENT;
}

// Create any parent directories above path, if they do not exist.
int create_parents(const char* path) {
    char* copy_path = strdup(path);
    char* dirname_path = dirname(copy_path);

    // If the parent of `path` exists, we don't need to do anything
    struct stat parent_stat = {0};
    errno = 0;
    int result = stat(dirname_path, &parent_stat);
    if (0 != result && (errno ^ ENOENT)) {
        perror("Couldn't check existence of parent directory");
        free(copy_path);
        return result;
    } else if (0 == result) {
        free(copy_path);
        return 0;
    }

    // Create any other parent path components before attempting to create the
    // parent of `path`.
    result = create_parents(dirname_path);
    if (0 != result) {
        free(copy_path);
        return result;
    }

    // Now that all parents exist, create this directory.
    result = mkdir(dirname_path, 0777);
    free(copy_path);
    if (0 != result) {
        perror("Couldn't create parent directory");
    }
    return result;
}

// Create the file, if it does not already exist.
int create_file(const char* path) {
    errno = 0;
    int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (0 > fd && (errno & EEXIST)) {
        fprintf(stderr, "File exists, ignoring request to create.\n");
    } else if (0 > fd) {
        perror("Couldn't create file");
        return 1;
    }

    close(fd);
    return 0;
}

int main(int argc, char** argv) {
    char* filename;
    argp_parse(&argp, argc, argv, 0, 0, &filename);

    int result = create_parents(filename);
    if (result) {
        return result;
    }

    result = create_file(filename);
    if (result) {
        return result;
    }

    // Locate git in PATH
    char git_path[PATH_MAX];
    result = find_git(git_path, PATH_MAX);
    if (0 != result) {
        perror("Couldn't find git executable in path");
        return errno;
    }

    char* args[] = {git_path, "add", filename, NULL};
    char* envp[] = {NULL};
    execve(git_path, args, envp);
}

///////////////////////////////////////////////////////////////////////////////
