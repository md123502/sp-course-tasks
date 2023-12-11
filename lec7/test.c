#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student_w_ops.h"
#include "students_array_w_ops.h"

bool predicate_to_delete(const student* s) {
    return s->grade_book_num == 55;
}

bool predicate_to_replace(const student* s) {
    return s->grade_book_num == 22;
}

void* group_numbers_sum(const student* s, void* acc) {
    void* result = acc;
    /**
     * Simple cast to void* can be unsafe if 
     * sizeof(actual_type) != sizeof(void*)
     * (even with numerical types it can happen, e.g., on 32-bit systems),
     * so pointer has to be a real pointer to normal allocated memory
     * if we want to avoid this
    */
    if (NULL == result) {
        result = malloc(sizeof(size_t));
        if (NULL == result) {
            return NULL; // It is enough for testing
        }
        *((size_t*)result) = 0;
    }
    size_t* result_casted = (size_t*) result;
    *result_casted += s->grade_book_num;
    return result_casted;
}

int main() {
    students_array* array = st_new_array(0);
    if (NULL == array) {
        fprintf(stderr, "st_new_array failed\n");
        return 1;
    }

    for (size_t i = 0; i < 5; ++i) {
        int result = st_interactive_add(array, stdin, stdout);
        if (0 != result) {
            sts_destroy_all(&array);
            fprintf(stderr, "st_interactive_add failed, code == %d\n", result);
            return 2;
        }
    }

    sts_formatted_print_all(array, stdout);

    FILE* file = fopen("./build/out.txt", "w");
    if (NULL == file) {
        fprintf(stderr, "Failed to open file\n");
        sts_destroy_all(&array);
        return 3;
    }

    sts_formatted_print_all(array, file);

    fclose(file);

    printf("Deleting elem\n");

    st_del_where(array, predicate_to_delete);

    sts_formatted_print_all(array, stdout);

    printf("Replacing elem\n");
    

    // These bufs will be freed on sts_destroy_all() call
    char* buf1 = (char*) malloc(9 * sizeof(*buf1));
    char* buf2 = (char*) malloc(9 * sizeof(*buf2));
    char* buf3 = (char*) malloc(3 * sizeof(*buf3));

    if (NULL == buf1 || NULL == buf2 || NULL == buf3) {
        fprintf(stderr, "Memory allocation failed for test strings\n");
        free(buf1);
        free(buf2);
        free(buf3);
    }

    strcpy(buf1, "REPLACED");
    strcpy(buf2, "replaced");
    strcpy(buf3, "35");

    student new_entry;
    new_entry.surname = buf1;
    new_entry.grade_book_num = 100;
    new_entry.faculty = buf2;
    new_entry.group = buf3;

    st_replace_where(array, predicate_to_replace, new_entry);

    sts_formatted_print_all(array, stdout);

    void* folding_result = sts_fold(array, group_numbers_sum);
    printf("Folding result: %zu\n", *((size_t*) folding_result));
    free(folding_result);

    sts_sort_surname_desc(array);

    printf("Surname desc\n");

    sts_formatted_print_all(array, stdout);

    sts_sort_surname_asc(array);

    printf("Surname asc\n");

    sts_formatted_print_all(array, stdout);

    sts_sort_grade_book_num_asc(array);

    printf("Book num asc\n");

    sts_formatted_print_all(array, stdout);

    sts_sort_grade_book_num_desc(array);

    printf("Book num desc\n");

    sts_formatted_print_all(array, stdout);

    sts_sort_faculty_asc(array);

    printf("Faculty asc\n");

    sts_formatted_print_all(array, stdout);

    sts_sort_faculty_desc(array);

    printf("Faculty desc\n");

    sts_formatted_print_all(array, stdout);

    sts_sort_group_asc(array);

    printf("Group asc\n");

    sts_formatted_print_all(array, stdout);

    sts_sort_group_desc(array);

    printf("Group desc\n");

    sts_formatted_print_all(array, stdout);

    student* student_with_surname = st_find_one_exact_surname(array, "def");

    printf("Student found: \n");

    st_formatted_print(student_with_surname, stdout);

    students_array* students_with_close_book = st_find_all_closest_grade_book_num(array, 22);

    sts_formatted_print_all(students_with_close_book, stdout);
    
    // Not calling sts_destroy_all on subarray to avoid double-free of strings

    free(students_with_close_book->students);
    free(students_with_close_book);

    sts_destroy_all(&array);
    
    printf("Finished\n");
}
