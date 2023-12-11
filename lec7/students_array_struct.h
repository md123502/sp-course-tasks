#ifndef STUDENTS_ARRAY_STRUCT_H
#define STUDENTS_ARRAY_STRUCT_H

#include <stddef.h>
#include "students_struct.h"

typedef struct students_array {
    student* students;
    size_t students_num;
    size_t capacity;
} students_array;

#endif
