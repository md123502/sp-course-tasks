#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "cli_text_defines.h"
#include "error_codes_enum.h"

#define PASSWD_FILE_PATH "/etc/passwd"

void skip_chars_until_char(FILE* stream, char stop_character) {
    assert(NULL != stream);
    int char_read = 0;
    do {
        char_read = fgetc(stream);
    } while ((EOF != char_read) && (stop_character != (char) char_read));
}

bool char_array_contains(const char* chars, size_t num, char character) {
    assert(NULL != chars);
    for (size_t i = 0; i < num; ++i) {
        if (chars[i] == character) {
            return true;
        }
    }
    return false;
}

char* append_char_to_arr(char character, size_t arr_len, char* arr) {
    char* realloc_result = realloc(arr, arr_len + 1);
    if (NULL != realloc_result) {
        realloc_result[arr_len] = character;
    }
    return realloc_result;
}

char* read_until_first_of(FILE* stream, const char* stop_chars, 
                          size_t chars_num, int* error_flag) {
    assert(NULL != stream);
    assert(NULL != stop_chars);
    assert(NULL != error_flag);
    char* result = NULL;
    size_t result_len = 0;
    int char_read = 0;
    bool allocation_error = false;
    do {
        char_read = fgetc(stream);
        if (EOF == char_read) {
            break;
        }
        if (!char_array_contains(stop_chars, chars_num, char_read)) {
            char* appending_result = 
                append_char_to_arr(char_read, result_len, result);
            if (NULL == appending_result) {
                allocation_error = true;
                break;
            }
            else {
                result_len++;
                result = appending_result;
            }
        }
        else {
            break;
        }
    } while (!ferror(stream) && !feof(stream));
    bool error = ferror(stream);
    bool eof = feof(stream);
    if (error || eof || allocation_error) {
        if (error) {
            *error_flag = READING_ERROR;
        }
        if (eof) {
            *error_flag = UNEXPECTED_EOF;
        }
        if (allocation_error) {
            *error_flag = MEMORY_ALLOCATION_ERROR;
        }
        free(result);
        return NULL;
    }
    // Empty name will be represented by single '\0'
    char* appending_result = append_char_to_arr('\0', result_len, result);
    if (NULL == appending_result) {
        *error_flag = MEMORY_ALLOCATION_ERROR;
        free(result);
        return NULL;
    }
    return appending_result;
}

char* get_username(FILE* passwd, const char* user_login, int* error_flag) {
    assert(NULL != passwd);
    assert(NULL != user_login);
    assert(NULL != error_flag);
    size_t login_len = strlen(user_login);
    /*
    As well as 1 additional byte for '\0', 
    we need 1 more for ':' (or other char if lenghts of names do not match)
    */
    char* buffer_for_login = malloc(login_len + 2);
    if (NULL == buffer_for_login) {
        *error_flag = MEMORY_ALLOCATION_ERROR;
        return NULL;
    }
    while (!feof(passwd) && !ferror(passwd)) {
        /* 
        If NULL == fgets(...), it's either feof before any chars or ferror;
        both cases are handled by loop's condition
        */
        if (NULL != fgets(buffer_for_login, login_len + 2, passwd)) {
            if (!strncmp(user_login, buffer_for_login, login_len)) {
                if (':' == buffer_for_login[login_len]) {
                    break;
                }
            }
            skip_chars_until_char(passwd, '\n');
        }
    }
    free(buffer_for_login);
    if (ferror(passwd)) {
        *error_flag = READING_ERROR;
        return NULL;
    }
    if (feof(passwd)) {
        *error_flag = USER_NOT_FOUND;
        return NULL;
    }
    // So here user is found
    const size_t field_number = 4;
    // We've already got first ':' from stream
    for (size_t i = 0; i < field_number - 1; ++i) {
        skip_chars_until_char(passwd, ':');
    }
    /* 
    For some users (root, bin, sys, ...) 4th field does not contain any commas
    */
    const size_t stop_chars_num = 2;
    char stop_chars[stop_chars_num];
    stop_chars[0] = ',';
    stop_chars[1] = ':';
    return read_until_first_of(passwd, stop_chars, stop_chars_num, error_flag);
}

int main(int argc, char** argv) {
    if (2 != argc) {
        printf(INPUT_HINT);
        return NOT_ENOUGH_ARGUMENTS;
    }
    FILE* passwd = fopen(PASSWD_FILE_PATH, "r");
    if (NULL == passwd) {
        printf(CANNOT_OPEN_PASSWD_FILE_MSG);
        return CANNOT_OPEN_PASSWD_FILE;
    }
    int error_flag = 0;
    char* username = get_username(passwd, argv[1], &error_flag);
    if (NULL == username) {
        printf(CANNOT_GET_USERNAME_MSG);
        if (USER_NOT_FOUND == error_flag) {
            printf(USER_NOT_FOUND_MSG);
        }
        if (MEMORY_ALLOCATION_ERROR == error_flag) {
            printf(MEMORY_ALLOCATION_ERROR_MSG);
        }
        if (READING_ERROR == error_flag) {
            printf(READING_ERROR_MSG);
        }
        if (UNEXPECTED_EOF == error_flag) {
            printf(UNEXPECTED_EOF_MSG);
        }
    }
    fclose(passwd);
    if (NULL != username) {
        printf("%s\n", username);
    }
    free(username);
    return error_flag;
}

