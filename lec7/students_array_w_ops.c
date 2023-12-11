#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "students_array_w_ops.h"
#include "student_w_ops.h"

/**
 * All general comments are in header file
*/

students_array* st_new_array(size_t initial_capacity) {
    students_array* result = (students_array*) malloc(sizeof(*result));
    if (NULL == result) {
        return NULL;
    }
    if (0 == initial_capacity) {
        result->students = NULL;
    }
    else {
        student* students_buf = 
            (student*) malloc(sizeof(*students_buf) * initial_capacity);
        if (NULL == students_buf) {
            free(result);
            return NULL;
        }
        result->students = students_buf;
    }
    result->capacity = initial_capacity;
    result->students_num = 0;
    return result;
}

int st_add(students_array* collection, student entry) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        // Collection is empty
        student* students_buf = 
            (student*) malloc(sizeof(*students_buf));
        if (NULL == students_buf) {
            return STS_MEM_ALLOC_ERROR;
        }
        collection->students = students_buf;
        collection->capacity = 1;
        collection->students_num = 0;
    }
    if (collection->students_num < collection->capacity) {
        (collection->students)[collection->students_num] = entry;
    }
    else {
        student* new_students_buf = 
            (student*) realloc(collection->students, 
                sizeof(*(collection->students))*(collection->capacity + 1));
        if (NULL == new_students_buf) {
            return STS_MEM_ALLOC_ERROR;
        }
        collection->students = new_students_buf;
        (collection->students)[collection->students_num] = entry;
        collection->capacity++;
    }
    collection->students_num++;
    return 0;
}

void sts_destroy_all(students_array** collection) {
    assert(NULL != collection);
    if (NULL == *collection) {
        return;
    }
    if (NULL != ((*collection)->students)) {
        student* students_arr = (*collection)->students;
        size_t students_num = (*collection)->students_num;
        for (size_t i = 0; i < students_num; ++i) {
            free(students_arr[i].surname);
            free(students_arr[i].faculty);
            free(students_arr[i].group);
        }
        free((*collection)->students);
    }
    free(*collection);
    *collection = NULL;
}

void st_del_where(students_array* collection, bool (*predicate)(const student*)) {
    assert(NULL != collection);
    assert(NULL != predicate);
    if ((NULL == collection->students) || (0 == collection->students_num)) {
        return;
    }
    student* students_arr = collection->students;
    for (size_t i = 0; i < collection->students_num; ++i) {
        if (predicate(students_arr + i)) {
            free(students_arr[i].surname);
            free(students_arr[i].faculty);
            free(students_arr[i].group);
            if (collection->students_num - 1 != i) {
                memmove(students_arr + i, students_arr + i + 1, 
                        sizeof(*(collection->students)) * (collection->students_num - 1 - i));
            }
            collection->students_num--;
            return;
        }
    }
}

void st_replace_where(students_array* collection, 
                     bool (*predicate)(const student*),
                     student new_entry) {
    assert(NULL != collection);
    assert(NULL != predicate);
    if ((NULL == collection->students) || (0 == collection->students_num)) {
        return;
    }
    student* students_arr = collection->students;
    for (size_t i = 0; i < collection->students_num; ++i) {
        if (predicate(students_arr + i)) {
            free(students_arr[i].surname);
            free(students_arr[i].faculty);
            free(students_arr[i].group);
            students_arr[i] = new_entry;
        }
    }
}

static int st_interactive_get(FILE* istream, FILE* ostream, student* entry_got) {
    assert(NULL != istream);
    assert(NULL != ostream);
    assert(NULL != entry_got);
    fprintf(ostream, "Enter surname:\n");
    size_t buf_len = 0;
    char* surname = NULL;
    size_t surname_len = getline(&surname, &buf_len, istream);
    /**
     * Assuming we are writing for Linux, 
     * we can use POSIX extension function getline()
     * in order to both avoid buffer overflow possible with scanf() 
     * and not write fgets()-realloc() loop
    */
    if (-1 == surname_len) {
        free(surname); // We have to do this even on failure, POSIX says
        return STS_READING_INPUT_ERROR;
    }
    if ('\n' == surname[surname_len - 1]) {
        surname[surname_len - 1] = '\0';
    }
    fprintf(ostream, "Enter grade book number:\n");
    char* grade_book_num_str = NULL;
    size_t grade_book_num_str_len = 
        getline(&grade_book_num_str, &buf_len, istream);
    if (-1 == grade_book_num_str_len) {
        free(surname);
        free(grade_book_num_str);
        return STS_READING_INPUT_ERROR;
    }
    if ('\n' == grade_book_num_str[grade_book_num_str_len - 1]) {
        grade_book_num_str[grade_book_num_str_len - 1] = '\0';
    }
    char* rest_of_converted_str = NULL;
    long long grade_book_num = 
        strtoll(grade_book_num_str, &rest_of_converted_str, 10);
    if ((0 > grade_book_num) || ('\0' != *rest_of_converted_str)) {
        free(surname);
        free(grade_book_num_str);
        return ST_INVALID_DATA;
    }
    free(grade_book_num_str);
    fprintf(ostream, "Enter faculty:\n");
    char* faculty = NULL;
    size_t faculty_len = getline(&faculty, &buf_len, istream);
    if (-1 == faculty_len) {
        free(surname);
        free(grade_book_num_str);
        free(faculty);
        return STS_READING_INPUT_ERROR;
    }
    if ('\n' == faculty[faculty_len - 1]) {
        faculty[faculty_len - 1] = '\0';
    }
    fprintf(ostream, "Enter group:\n");
    char* group = NULL;
    size_t group_len = getline(&group, &buf_len, istream);
    if (-1 == group_len) {
        free(surname);
        free(grade_book_num_str);
        free(faculty);
        free(group);
        return STS_READING_INPUT_ERROR;
    }
    if ('\n' == group[group_len - 1]) {
        group[group_len - 1] = '\0';
    }
    entry_got->surname = surname;
    entry_got->grade_book_num = grade_book_num;
    entry_got->faculty = faculty;
    entry_got->group = group;
    return 0;
}

int st_interactive_add(students_array* collection, FILE* istream, FILE* ostream) {
    assert(NULL != collection);
    assert(NULL != istream);
    assert(NULL != ostream);
    student new_entry;
    int getting_result = st_interactive_get(istream, ostream, &new_entry);
    if (0 != getting_result) {
        return getting_result;
    }
    return st_add(collection, new_entry);
}

void sts_formatted_print_all(const students_array* collection, FILE* ostream) {
    assert(NULL != collection);
    assert(NULL != ostream);
    if ((NULL == collection->students) || (0 == collection->students_num)) {
        return;
    }
    fprintf(ostream, "students_array collection:\n");
    fprintf(ostream, "students_num in collection == %zu:\n", 
            collection->students_num);
    fprintf(ostream, "capacity of collection == %zu:\n", collection->capacity);
    for (size_t i = 0; i < collection->students_num; ++i) {
        st_formatted_print(collection->students + i, ostream);
    }
}

void* sts_fold(students_array* collection, 
                void* operation(const student*, void*)) {
    assert(NULL != collection);
    assert(NULL != operation);
    if (NULL == collection->students) {
        return NULL;
    }
    void* accumulator = NULL;
    for (size_t i = 0; i < collection->students_num; ++i) {
        accumulator = operation(collection->students + i, accumulator);
    }
    return accumulator;
}



/*************** Beginning of sort functions ***************/

void sts_sort_any(const students_array* collection, 
                  int (*comparator)(const void*, const void*)) {
    assert(NULL != collection);
    assert(NULL != comparator);
    if (NULL == collection->students) {
        return;
    }
    qsort(collection->students, collection->students_num, 
          sizeof(*(collection->students)), comparator);
}

static int st_comparator_surname_asc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return strcmp(s1->surname, s2->surname);
}

void sts_sort_surname_asc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_surname_asc);
}

static int st_comparator_surname_desc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return -strcmp(s1->surname, s2->surname);
}

void sts_sort_surname_desc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_surname_desc);
}

static int st_comparator_grade_book_num_asc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return (s1->grade_book_num - s2->grade_book_num);
}

void sts_sort_grade_book_num_asc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_grade_book_num_asc);
}

static int st_comparator_grade_book_num_desc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return (s2->grade_book_num - s1->grade_book_num);
}

void sts_sort_grade_book_num_desc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_grade_book_num_desc);
}

static int st_comparator_faculty_asc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return strcmp(s1->faculty, s2->faculty);
}

void sts_sort_faculty_asc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_faculty_asc);
}

static int st_comparator_faculty_desc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return -strcmp(s1->faculty, s2->faculty);
}

void sts_sort_faculty_desc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_faculty_desc);
}

static int st_comparator_group_asc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return strcmp(s1->group, s2->group);
}

void sts_sort_group_asc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_group_asc);
}

static int st_comparator_group_desc(const void* arg1, const void* arg2) {
    assert(NULL != arg1);
    assert(NULL != arg2);
    student* s1 = (student*) arg1;
    student* s2 = (student*) arg2;
    return -strcmp(s1->group, s2->group);
}

void sts_sort_group_desc(const students_array* collection) {
    assert(NULL != collection);
    if (NULL == collection->students) {
        return;
    }
    sts_sort_any(collection, st_comparator_group_desc);
}

/****************** End of sort functions ******************/



/************** Beginning of search functions **************/

student* st_find_one_closest_any(const students_array* collection, 
                                 int (*distance)(const student*, const void*), 
                                 const void* arg) {
    assert(NULL != collection);
    assert(NULL != distance);
    if ((NULL == collection->students) || (0 == collection->students_num)) {
        return NULL;
    }
    int min_distance = distance(collection->students, arg);
    student* result = collection->students;
    for (size_t i = 1; i < collection->students_num; ++i) {
        int cur_distance = distance(collection->students + i, arg);
        if (min_distance > cur_distance) {
            result = collection->students + i;
            min_distance = cur_distance;
        }
    }
    return result;
}

student* st_find_one_exact_any(const students_array* collection, 
                               int (*distance)(const student*, const void*), 
                               const void* arg) {
    assert(NULL != collection);
    assert(NULL != distance);
    if ((NULL == collection->students) || (0 == collection->students_num)) {
        return NULL;
    }
    for (size_t i = 0; i < collection->students_num; ++i) {
        int cur_distance = distance(collection->students + i, arg);
        if (0 == cur_distance) {
            return collection->students + i;
        }
    }
    return NULL;
}

students_array* st_find_all_closest_any(
                                const students_array* collection, 
                                int (*distance)(const student*, const void*), 
                                const void* arg) {
    assert(NULL != collection);
    assert(NULL != distance);
    students_array* result = st_new_array(0);
    if ((NULL == collection->students) || (0 == collection->students_num)) {
        return result;
    }
    int min_distance = distance(collection->students, arg);
    for (size_t i = 1; i < collection->students_num; ++i) {
        int cur_distance = distance(collection->students + i, arg);
        if (min_distance > cur_distance) {
            min_distance = cur_distance;
        }
    }
    for (size_t i = 0; i < collection->students_num; ++i) {
        int cur_distance = distance(collection->students + i, arg);
        if (min_distance == cur_distance) {
            if (st_add(result, (collection->students)[i])) {
                /**
                 * We did not duplicate strings, 
                 * so should not call sts_destroy_all(result) 
                */
                free(result->students);
                free(result);
                return NULL;
            }
        }
    }
    return result;
}

students_array* st_find_all_exact_any(
                                const students_array* collection, 
                                int (*distance)(const student*, const void*), 
                                const void* arg) {
    assert(NULL != collection);
    assert(NULL != distance);
    students_array* result = st_new_array(0);
    if ((NULL == collection->students) || (0 == collection->students_num)) {
        return result;
    }
    for (size_t i = 0; i < collection->students_num; ++i) {
        int cur_distance = distance(collection->students + i, arg);
        if (0 == cur_distance) {
            if (st_add(result, (collection->students)[i])) {
                /**
                 * We did not duplicate strings, 
                 * so should not call sts_destroy_all(result) 
                */
                free(result->students);
                free(result);
                return NULL;
            }
        }
    }
    return result;
}

static int st_distance_surname(const student* student, const void* value) {
    assert(NULL != student);
    assert(NULL != value);
    char* surname = (char*) value;
    return abs(strcmp(student->surname, surname));
}

student* st_find_one_closest_surname(const students_array* collection, 
                                     const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_one_closest_any(collection, st_distance_surname, value);
}

student* st_find_one_exact_surname(const students_array* collection, 
                                   const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_one_exact_any(collection, st_distance_surname, value);
}

static int st_distance_grade_book_num(const student* student, const void* value) {
    assert(NULL != student);
    assert(NULL != value);
    size_t* grade_book_num = (size_t*) value;
    return abs(student->grade_book_num - *grade_book_num);
}

student* st_find_one_closest_grade_book_num(const students_array* collection, 
                                            size_t value) {
    assert(NULL != collection);
    size_t saved_val = value;
    return st_find_one_closest_any(collection, st_distance_grade_book_num, &saved_val);
}

student* st_find_one_exact_grade_book_num(const students_array* collection, 
                                          size_t value) {
    assert(NULL != collection);
    size_t saved_val = value;
    return st_find_one_exact_any(collection, st_distance_grade_book_num, &saved_val);
}

static int st_distance_faculty(const student* student, const void* value) {
    assert(NULL != student);
    assert(NULL != value);
    char* faculty = (char*) value;
    return abs(strcmp(student->faculty, faculty));
}

student* st_find_one_closest_faculty(const students_array* collection, 
                                     const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_one_closest_any(collection, st_distance_faculty, value);
}

student* st_find_one_exact_faculty(const students_array* collection, 
                                   const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_one_exact_any(collection, st_distance_faculty, value);
}

static int st_distance_group(const student* student, const void* value) {
    assert(NULL != student);
    assert(NULL != value);
    char* group = (char*) value;
    return abs(strcmp(student->group, group));
}

student* st_find_one_closest_group(const students_array* collection, 
                                     const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_one_closest_any(collection, st_distance_group, value);
}

student* st_find_one_exact_group(const students_array* collection, 
                                   const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_one_exact_any(collection, st_distance_group, value);
}

students_array* st_find_all_closest_surname(const students_array* collection, 
                                            const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_all_closest_any(collection, st_distance_surname, value);
}

students_array* st_find_all_exact_surname(const students_array* collection, 
                                          const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_all_exact_any(collection, st_distance_surname, value);
}

students_array* st_find_all_closest_grade_book_num(const students_array* collection, 
                                                   size_t value) {
    assert(NULL != collection);
    size_t saved_val = value;
    return st_find_all_closest_any(collection, st_distance_grade_book_num, &saved_val);
}

students_array* st_find_all_exact_grade_book_num(const students_array* collection, 
                                                 size_t value) {
    assert(NULL != collection);
    size_t saved_val = value;
    return st_find_all_exact_any(collection, st_distance_grade_book_num, &saved_val);
}

students_array* st_find_all_closest_faculty(const students_array* collection, 
                                     const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_all_closest_any(collection, st_distance_faculty, value);
}

students_array* st_find_all_exact_faculty(const students_array* collection, 
                                   const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_all_exact_any(collection, st_distance_faculty, value);
}

students_array* st_find_all_closest_group(const students_array* collection, 
                                     const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_all_closest_any(collection, st_distance_group, value);
}

students_array* st_find_all_exact_group(const students_array* collection, 
                                   const char* value) {
    assert(NULL != collection);
    assert(NULL != value);
    return st_find_all_exact_any(collection, st_distance_group, value);
}

/***************** End of search functions *****************/
