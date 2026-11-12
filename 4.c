// Structures - 1 (Extended: Student Records and Academic Analytics System)
/*
 * Task Description:
 *   Extends the dynamically allocated Student array program into a menu-driven
 *   student records management and academic analytics system.
 *   - Retains core allocation, assignment, display, and cleanup helpers while
 *     expanding Student to store a unique positive roll_number, name (up to 49
 *     characters + '\0'), age (16..100), and GPA (0.00..4.00).
 *   - Tracks separate record count and allocated capacity, growing dynamically
 *     via realloc with arithmetic overflow checks and rollback safety if
 *     allocation fails.
 *   - Supports add, display, search by roll number, update, delete (preserving
 *     order of remaining records), GPA summary (highest, lowest, average), and
 *     sorted display (descending GPA, tie-broken by ascending roll number).
 *   - Sorting operates on a temporary copy so the original insertion order in
 *     the primary array remains intact.
 *
 * Compilation:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic 4.c -o 4
 *
 * Sample Run:
 *   Enter initial student capacity (1 - 10000): 2
 *   Initialized student list with capacity 2.
 *   Choice> 1 (Add) -> Roll: 101, Name: "Aladdin", Age: 20, GPA: 3.50 -> Added (1/2)
 *   Choice> 1 (Add) -> Roll: 102, Name: "Bruce Wayne", Age: 21, GPA: 3.70 -> Added (2/2)
 *   Choice> 1 (Add) -> Roll: 103, Name: "Selina Kyle", Age: 22, GPA: 3.80
 *           -> Capacity expanded from 2 to 4. Added (3/4)
 *   Choice> 3 (Search) -> Roll: 102 -> Roll: 102, Name: Bruce Wayne, Age: 21, GPA: 3.70
 *   Choice> 4 (Update) -> Roll: 102 -> Name: "Bruce Wayne", Age: 21, GPA: 3.90 -> Updated
 *   Choice> 5 (Delete) -> Roll: 101 -> Deleted Roll 101 (Count: 2/4)
 *   Choice> 6 (Sorted View) ->
 *           Roll: 102 | Name: Bruce Wayne | Age: 21 | GPA: 3.90
 *           Roll: 103 | Name: Selina Kyle | Age: 22 | GPA: 3.80
 *   Choice> 7 (GPA Summary) ->
 *           Highest GPA: 3.90 (Roll 102 - Bruce Wayne)
 *           Lowest GPA : 3.80 (Roll 103 - Selina Kyle)
 *           Average GPA: 3.85
 *   Choice> 0 (Exit) -> Freed student list. Exiting.
 *
 * Edge Cases Tested:
 *   1. Empty collection: Display, search, update, delete, sort, and summary
 *      report clear messages without reading invalid memory.
 *   2. Invalid initial capacity: Rejects <= 0, > 10000, or non-integer text.
 *   3. Duplicate roll number: Rejected on add before mutating the array.
 *   4. Invalid age (< 16 or > 100) and invalid GPA (< 0.00, > 4.00, NaN, Inf):
 *      Rejected with explicit range messages.
 *   5. Maximum-length name (49 chars) & names with spaces: Accepted cleanly;
 *      names >= 50 chars are detected as overlong, buffer flushed, and rejected.
 *   6. Search / update / delete of non-existent roll number: Handled gracefully.
 *   7. Dynamic reallocation: Triggered when count == capacity; preserves existing
 *      pointer and records if overflow check or realloc fails.
 */

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_MAX_LEN 49
#define NAME_BUF_SIZE 50
#define LINE_BUF_SIZE 256
#define MAX_INITIAL_CAPACITY 10000
#define MIN_AGE 16
#define MAX_AGE 100
#define MIN_GPA 0.0f
#define MAX_GPA 4.0f
#define GPA_EPSILON 1e-5f

typedef struct {
    int roll_number;
    char name[NAME_BUF_SIZE];
    int age;
    float gpa;
} Student;

/* --- Core Allocation, Assignment, Display & Cleanup Functions --- */

Student *create_student_list(int n) {
    if (n <= 0) {
        return NULL;
    }

    size_t count = (size_t)n;
    if (count > SIZE_MAX / sizeof(Student)) {
        return NULL;
    }

    Student *studs = malloc(count * sizeof(Student));

    return studs;
}

bool resize_student_list(Student **studs, int *capacity) {
    if (studs == NULL || capacity == NULL || *capacity <= 0) {
        return false;
    }

    if (*capacity > INT_MAX / 2) {
        return false;
    }

    int new_capacity = (*capacity) * 2;
    size_t new_count = (size_t)new_capacity;
    if (new_count > SIZE_MAX / sizeof(Student)) {
        return false;
    }

    Student *resized = realloc(*studs, new_count * sizeof(Student));
    if (resized == NULL) {
        return false;
    }

    *studs = resized;
    *capacity = new_capacity;
    printf("Notice: Capacity dynamically expanded to %d records.\n", *capacity);

    return true;
}

void add_student(Student *studs, int i, int roll_number, const char *name, int age, float gpa) {
    if (studs == NULL || name == NULL || i < 0) {
        return;
    }

    studs[i].roll_number = roll_number;
    strncpy(studs[i].name, name, NAME_MAX_LEN);
    studs[i].name[NAME_MAX_LEN] = '\0';
    studs[i].age = age;
    studs[i].gpa = gpa;
}

void display_students(const Student *students, int size) {
    if (students == NULL || size <= 0) {
        printf("Notice: No student records to display.\n");
        return;
    }

    printf("\n%-10s | %-49s | %-5s | %-6s\n", "Roll No.", "Name", "Age", "GPA");
    printf("-----------+---------------------------------------------------+-------+-------\n");

    for (int i = 0; i < size; i++) {
        printf("%-10d | %-49s | %-5d | %-6.2f\n",
               students[i].roll_number,
               students[i].name,
               students[i].age,
               (double)students[i].gpa);
    }
    printf("\n");
}

void free_student_list(Student *students) {
    free(students);
}

/* --- Input & Validation Helpers --- */

void flush_input_line(void) {
    int ch = 0;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

bool read_trimmed_line(char *buffer, size_t buf_size, bool *is_overlong) {
    if (is_overlong != NULL) {
        *is_overlong = false;
    }

    if (fgets(buffer, (int)buf_size, stdin) == NULL) {
        return false;
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    } else {
        int next_ch = getchar();
        if (next_ch != '\n' && next_ch != EOF) {
            flush_input_line();
            if (is_overlong != NULL) {
                *is_overlong = true;
            }
            return true;
        }
    }

    while (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == ' ' || buffer[len - 1] == '\t')) {
        buffer[len - 1] = '\0';
        len--;
    }

    return true;
}

bool read_int_range(const char *prompt, int min_val, int max_val, int *out_val) {
    char line[LINE_BUF_SIZE];
    bool is_overlong = false;

    printf("%s", prompt);
    if (!read_trimmed_line(line, sizeof(line), &is_overlong)) {
        return false;
    }

    if (is_overlong || line[0] == '\0') {
        printf("Error: Invalid integer input.\n");
        return false;
    }

    char *end_ptr = NULL;
    errno = 0;
    long parsed = strtol(line, &end_ptr, 10);

    while (*end_ptr == ' ' || *end_ptr == '\t') {
        end_ptr++;
    }

    if (errno == ERANGE || *end_ptr != '\0' || parsed < (long)min_val || parsed > (long)max_val) {
        printf("Error: Enter an integer between %d and %d.\n", min_val, max_val);
        return false;
    }

    *out_val = (int)parsed;

    return true;
}

bool read_valid_gpa(const char *prompt, float *out_gpa) {
    char line[LINE_BUF_SIZE];
    bool is_overlong = false;

    printf("%s", prompt);
    if (!read_trimmed_line(line, sizeof(line), &is_overlong)) {
        return false;
    }

    if (is_overlong || line[0] == '\0') {
        printf("Error: Invalid GPA input.\n");
        return false;
    }

    char *end_ptr = NULL;
    errno = 0;
    float parsed = strtof(line, &end_ptr);

    while (*end_ptr == ' ' || *end_ptr == '\t') {
        end_ptr++;
    }

    if (errno == ERANGE || *end_ptr != '\0' || !isfinite(parsed) ||
        parsed < MIN_GPA || parsed > MAX_GPA) {
        printf("Error: GPA must be a finite number between %.2f and %.2f.\n",
               (double)MIN_GPA, (double)MAX_GPA);
        return false;
    }

    *out_gpa = parsed;

    return true;
}

bool read_valid_name(const char *prompt, char out_name[NAME_BUF_SIZE]) {
    char line[LINE_BUF_SIZE];
    bool is_overlong = false;

    printf("%s", prompt);
    if (!read_trimmed_line(line, sizeof(line), &is_overlong)) {
        return false;
    }

    if (is_overlong) {
        printf("Error: Name exceeds the %d-character limit.\n", NAME_MAX_LEN);
        return false;
    }

    const char *start = line;
    while (*start == ' ' || *start == '\t') {
        start++;
    }

    size_t len = strlen(start);
    if (len == 0) {
        printf("Error: Student name cannot be empty.\n");
        return false;
    }

    if (len > NAME_MAX_LEN) {
        printf("Error: Name length (%zu) exceeds maximum allowed (%d characters).\n",
               len, NAME_MAX_LEN);
        return false;
    }

    memcpy(out_name, start, len + 1);

    return true;
}

/* --- Records Management Operations --- */

int find_student_index(const Student *students, int size, int roll_number) {
    if (students == NULL || size <= 0) {
        return -1;
    }

    for (int i = 0; i < size; i++) {
        if (students[i].roll_number == roll_number) {
            return i;
        }
    }

    return -1;
}

bool handle_add_student(Student **students, int *count, int *capacity) {
    if (students == NULL || count == NULL || capacity == NULL) {
        return false;
    }

    int roll_number = 0;
    if (!read_int_range("Enter unique positive roll number: ", 1, INT_MAX, &roll_number)) {
        return false;
    }

    if (find_student_index(*students, *count, roll_number) != -1) {
        printf("Error: Roll number %d already exists.\n", roll_number);
        return false;
    }

    char name[NAME_BUF_SIZE];
    if (!read_valid_name("Enter student name (max 49 chars): ", name)) {
        return false;
    }

    int age = 0;
    if (!read_int_range("Enter age (16 - 100): ", MIN_AGE, MAX_AGE, &age)) {
        return false;
    }

    float gpa = 0.0f;
    if (!read_valid_gpa("Enter GPA (0.00 - 4.00): ", &gpa)) {
        return false;
    }

    if (*count >= *capacity) {
        if (!resize_student_list(students, capacity)) {
            printf("Error: Memory reallocation failed; existing records preserved.\n");
            return false;
        }
    }

    add_student(*students, *count, roll_number, name, age, gpa);
    (*count)++;
    printf("Success: Added '%s' (Roll %d). [Count: %d/%d]\n",
           name, roll_number, *count, *capacity);

    return true;
}

bool handle_search_student(const Student *students, int count) {
    if (count <= 0) {
        printf("Notice: Student list is empty.\n");
        return false;
    }

    int roll_number = 0;
    if (!read_int_range("Enter roll number to search: ", 1, INT_MAX, &roll_number)) {
        return false;
    }

    int idx = find_student_index(students, count, roll_number);
    if (idx < 0) {
        printf("Error: Student with roll number %d not found.\n", roll_number);
        return false;
    }

    printf("Found -> Roll: %d, Name: %s, Age: %d, GPA: %.2f\n",
           students[idx].roll_number,
           students[idx].name,
           students[idx].age,
           (double)students[idx].gpa);

    return true;
}

bool handle_update_student(Student *students, int count) {
    if (count <= 0) {
        printf("Notice: Student list is empty.\n");
        return false;
    }

    int roll_number = 0;
    if (!read_int_range("Enter roll number to update: ", 1, INT_MAX, &roll_number)) {
        return false;
    }

    int idx = find_student_index(students, count, roll_number);
    if (idx < 0) {
        printf("Error: Student with roll number %d not found.\n", roll_number);
        return false;
    }

    printf("Current -> Roll: %d, Name: %s, Age: %d, GPA: %.2f\n",
           students[idx].roll_number,
           students[idx].name,
           students[idx].age,
           (double)students[idx].gpa);

    char new_name[NAME_BUF_SIZE];
    if (!read_valid_name("Enter new name (max 49 chars): ", new_name)) {
        return false;
    }

    int new_age = 0;
    if (!read_int_range("Enter new age (16 - 100): ", MIN_AGE, MAX_AGE, &new_age)) {
        return false;
    }

    float new_gpa = 0.0f;
    if (!read_valid_gpa("Enter new GPA (0.00 - 4.00): ", &new_gpa)) {
        return false;
    }

    add_student(students, idx, roll_number, new_name, new_age, new_gpa);
    printf("Success: Updated record for roll number %d.\n", roll_number);

    return true;
}

bool handle_delete_student(Student *students, int *count) {
    if (count == NULL || *count <= 0) {
        printf("Notice: Student list is empty.\n");
        return false;
    }

    int roll_number = 0;
    if (!read_int_range("Enter roll number to delete: ", 1, INT_MAX, &roll_number)) {
        return false;
    }

    int idx = find_student_index(students, *count, roll_number);
    if (idx < 0) {
        printf("Error: Student with roll number %d not found.\n", roll_number);
        return false;
    }

    for (int i = idx; i < (*count) - 1; i++) {
        students[i] = students[i + 1];
    }

    (*count)--;
    printf("Success: Deleted student with roll number %d. [Remaining: %d]\n",
           roll_number, *count);

    return true;
}

/* --- Sorting & Academic Summary --- */

int compare_students_gpa_desc(const void *lhs, const void *rhs) {
    const Student *a = (const Student *)lhs;
    const Student *b = (const Student *)rhs;

    float diff = b->gpa - a->gpa;
    if (diff > GPA_EPSILON) {
        return 1;
    }
    if (diff < -GPA_EPSILON) {
        return -1;
    }

    if (a->roll_number < b->roll_number) {
        return -1;
    }
    if (a->roll_number > b->roll_number) {
        return 1;
    }

    return 0;
}

/*
 * Sorting Choice Documentation:
 *   display_sorted_students allocates a temporary copy of the active array
 *   and sorts the copy via qsort so that the original storage order of the
 *   primary array is preserved without side effects.
 */
bool display_sorted_students(const Student *students, int count) {
    if (students == NULL || count <= 0) {
        printf("Notice: Student list is empty; nothing to sort.\n");
        return false;
    }

    Student *copy = create_student_list(count);
    if (copy == NULL) {
        printf("Error: Failed to allocate memory for sorted display.\n");
        return false;
    }

    memcpy(copy, students, (size_t)count * sizeof(Student));
    qsort(copy, (size_t)count, sizeof(Student), compare_students_gpa_desc);

    printf("\nStudents Sorted by GPA (Descending), Ties Broken by Roll Number (Ascending):");
    display_students(copy, count);

    free_student_list(copy);

    return true;
}

bool display_gpa_summary(const Student *students, int count) {
    if (students == NULL || count <= 0) {
        printf("Notice: Student list is empty; no GPA summary available.\n");
        return false;
    }

    int highest_idx = 0;
    int lowest_idx = 0;
    double total_gpa = 0.0;

    for (int i = 0; i < count; i++) {
        total_gpa += (double)students[i].gpa;

        if (students[i].gpa > students[highest_idx].gpa) {
            highest_idx = i;
        }
        if (students[i].gpa < students[lowest_idx].gpa) {
            lowest_idx = i;
        }
    }

    double average_gpa = total_gpa / (double)count;

    printf("\n--- Academic GPA Summary (%d Students) ---\n", count);
    printf("Highest GPA : %.2f (Roll %d - %s)\n",
           (double)students[highest_idx].gpa,
           students[highest_idx].roll_number,
           students[highest_idx].name);
    printf("Lowest GPA  : %.2f (Roll %d - %s)\n",
           (double)students[lowest_idx].gpa,
           students[lowest_idx].roll_number,
           students[lowest_idx].name);
    printf("Average GPA : %.2f\n\n", average_gpa);

    return true;
}

void print_menu(void) {
    printf("=== Student Records & Academic Analytics ===\n");
    printf("1. Add Student Record\n");
    printf("2. Display All Students\n");
    printf("3. Search Student by Roll Number\n");
    printf("4. Update Student Record\n");
    printf("5. Delete Student Record\n");
    printf("6. Display Students Sorted by GPA\n");
    printf("7. Display GPA Summary\n");
    printf("0. Exit\n");
}

int main(void) {
    int capacity = 0;
    while (true) {
        if (feof(stdin)) {
            printf("Input closed before initialization. Exiting.\n");
            return 1;
        }

        if (read_int_range("Enter initial student capacity (1 - 10000): ",
                           1, MAX_INITIAL_CAPACITY, &capacity)) {
            break;
        }
    }

    Student *student_list = create_student_list(capacity);
    if (student_list == NULL) {
        printf("Fatal Error: Memory allocation failed for capacity %d.\n", capacity);
        return 1;
    }

    int count = 0;
    printf("Initialized student list with capacity %d.\n\n", capacity);

    bool running = true;
    while (running) {
        if (feof(stdin)) {
            printf("\nEnd of input reached. Releasing memory and exiting.\n");
            break;
        }

        print_menu();
        int choice = -1;
        if (!read_int_range("Select an option (0 - 7): ", 0, 7, &choice)) {
            continue;
        }

        switch (choice) {
            case 1: {
                handle_add_student(&student_list, &count, &capacity);
                break;
            }
            case 2: {
                printf("\nStored Records (%d / %d capacity):", count, capacity);
                display_students(student_list, count);
                break;
            }
            case 3: {
                handle_search_student(student_list, count);
                break;
            }
            case 4: {
                handle_update_student(student_list, count);
                break;
            }
            case 5: {
                handle_delete_student(student_list, &count);
                break;
            }
            case 6: {
                display_sorted_students(student_list, count);
                break;
            }
            case 7: {
                display_gpa_summary(student_list, count);
                break;
            }
            case 0: {
                running = false;
                break;
            }
            default: {
                printf("Error: Unknown option.\n");
                break;
            }
        }
    }

    free_student_list(student_list);
    printf("Memory released. Program terminated cleanly.\n");

    return 0;
}
