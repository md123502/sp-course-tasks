#include <stdio.h>
#include <assert.h>

#include "student_w_ops.h"

#define FORMAT_STRING "Student:\n\t surname: %s\n\t grade_book_num: %d\n\t\
 faculty: %s\n\t group: %s\n"

void st_formatted_print(const student* s, FILE* ostream) {
    assert(NULL != s);
    assert(NULL != ostream);
    fprintf(ostream, FORMAT_STRING, s->surname, s->grade_book_num, 
            s->faculty, s->group);
}
