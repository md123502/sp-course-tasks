#ifndef STUDENTS_ARRAY_W_OPS_H
#define STUDENTS_ARRAY_W_OPS_H

#include <stdio.h>
#include <stdbool.h>
#include "students_array_struct.h"

enum students_array_ops_return_codes {
    STS_MEM_ALLOC_ERROR = 1,
    STS_READING_INPUT_ERROR,
    ST_INVALID_DATA,
};

/**
 * If 'initial_capacity' arg is 0, 
 *  returns students_array* with 'students' field set to NULL
 *  and consistent values in other fields
 * If memory allocation fails, returns NULL
*/
students_array* st_new_array(size_t initial_capacity);

int st_add(students_array* collection, student entry);

/**
 * After freeing data, sets *collection to NULL
*/
void sts_destroy_all(students_array** collection);

void st_del_where(students_array* collection, bool (*predicate)(const student*));

/**
 * Replaces all entries where predicate(entry) is true
*/
void st_replace_where(students_array* collection, 
                     bool (*predicate)(const student*),
                     student new_entry);

int st_interactive_add(students_array* collection, FILE* istream, FILE* ostream);

void sts_formatted_print_all(const students_array* collection, FILE* ostream);

/**
 * Function allowing to calculate a single value based on whole array
 *  (smth like fold() in functional programming)
 * ...hope I understood correctly what you wanted to say in last point of task
*/
void* sts_fold(students_array* collection, 
                void* operation(const student*, void*));

/*************** Beginning of sort functions ***************/

/** 
 * comparator()'s arg type is 'const void*' and not 'const student*'
 *  in order to match libc's qsort() definition
*/
void sts_sort_any(const students_array* collection, 
                  int (*comparator)(const void*, const void*));

/**
 * Some predefined sorts by field for convenience
 * For strings order is determined by strcmp() implementation 
*/
void sts_sort_surname_asc(const students_array* collection);
void sts_sort_surname_desc(const students_array* collection);
void sts_sort_grade_book_num_asc(const students_array* collection);
void sts_sort_grade_book_num_desc(const students_array* collection);
void sts_sort_faculty_asc(const students_array* collection);
void sts_sort_faculty_desc(const students_array* collection);
void sts_sort_group_asc(const students_array* collection);
void sts_sort_group_desc(const students_array* collection);

/****************** End of sort functions ******************/



/************** Beginning of search functions **************/

/**
 * 'const void* arg' is GNU's qsort_r()-style parameter
 * Returns entry with minimal distance(entry, arg)
 *  (first, if there are more than one), 
 *  NULL can be returned only if there are no elems at all
*/
student* st_find_one_closest_any(const students_array* collection, 
                                 int (*distance)(const student*, const void*), 
                                 const void* arg);

/**
 * 'const void* arg' is GNU's qsort_r()-style parameter
 * Returns entry for which (0 == distance(entry, arg)) 
 *  (first, if there are more than one) or NULL if such one doesn't exist
*/
student* st_find_one_exact_any(const students_array* collection, 
                               int (*distance)(const student*, const void*), 
                               const void* arg);

/**
 * See st_find_one* functions' comments above for details.
 * st_find_all* functions' only difference is that they return
 *  students_array* - subarray of matching elements 
 *  instead of the first occuring one.
*/
students_array* st_find_all_closest_any(
                                const students_array* collection, 
                                int (*distance)(const student*, const void*), 
                                const void* arg);


students_array* st_find_all_exact_any(
                                const students_array* collection, 
                                int (*distance)(const student*, const void*), 
                                const void* arg);

/**
 * Some predefined find()'s by field for convenience
 * 
 * For strings distance is determined by an absolute value of strcmp() result.
 *  For glibc and most other implementations
 *   it is 0 or first chars subtraction result that is non-zero.
 *  Consequently, 'John' will be further from 'john' than from 'Bohn'
 *   due to the fact that in ASCII all capital letters precede all usual ones.
 *  However, the task does not specify certain way of distance calculation,
 *   so I consider it normal.
 * 
*/
student* st_find_one_closest_surname(const students_array* collection, 
                                     const char* value);
student* st_find_one_exact_surname(const students_array* collection, 
                                   const char* value);
student* st_find_one_closest_grade_book_num(const students_array* collection, 
                                            size_t value);
student* st_find_one_exact_grade_book_num(const students_array* collection, 
                                          size_t value);
student* st_find_one_closest_faculty(const students_array* collection, 
                                     const char* value);
student* st_find_one_exact_faculty(const students_array* collection, 
                                   const char* value);
student* st_find_one_closest_group(const students_array* collection, 
                                   const char* value);
student* st_find_one_exact_group(const students_array* collection, 
                                 const char* value);

students_array* st_find_all_closest_surname(const students_array* collection, 
                                            const char* value);
students_array* st_find_all_exact_surname(const students_array* collection, 
                                          const char* value);
students_array* st_find_all_closest_grade_book_num(
                                        const students_array* collection, 
                                        size_t value);
students_array* st_find_all_exact_grade_book_num(
                                        const students_array* collection, 
                                        size_t value);
students_array* st_find_all_closest_faculty(const students_array* collection, 
                                            const char* value);
students_array* st_find_all_exact_faculty(const students_array* collection, 
                                          const char* value);
students_array* st_find_all_closest_group(const students_array* collection, 
                                          const char* value);
students_array* st_find_all_exact_group(const students_array* collection, 
                                        const char* value);


/***************** End of search functions *****************/

#endif
