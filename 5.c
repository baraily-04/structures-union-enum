// Structures - 2 (Extended: Rectangle Inventory and Geometry Analysis)
/*
 * Task Description:
 *   Extends the dynamically allocated Rectangle array program into an interactive
 *   geometry inventory and analysis system.
 *   - Retains the core Rectangle structure and helper functions while expanding
 *     Rectangle to store a unique positive id, width, height, area, and perimeter.
 *   - Validates runtime initial capacity (1..1000) and finite positive dimensions
 *     (rejecting 0, negative values, NaN, and infinity).
 *   - Recomputes area and perimeter automatically upon insertion or update.
 *   - Supports add (with overflow-safe realloc growth), display, search by ID,
 *     update dimensions by ID, delete by ID, square classification via relative
 *     floating-point tolerance, sorting by ascending area (breaking ties by
 *     ascending ID) on a temporary copy, area-threshold filtering, and
 *     summary statistics (largest, smallest, total, and average area).
 *
 * Compilation:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic 5.c -o 5
 *
 * Sample Run:
 *   Enter initial rectangle capacity (1 - 1000): 2
 *   Initialized rectangle array with capacity 2.
 *   Choice> 1 (Add) -> ID: 201, Width: 5.0, Height: 4.0 -> Added (Area=20.00, Perim=18.00, Non-square)
 *   Choice> 1 (Add) -> ID: 202, Width: 6.5, Height: 3.5 -> Added (Area=22.75, Perim=20.00, Non-square)
 *   Choice> 1 (Add) -> ID: 203, Width: 4.0, Height: 4.0 -> Resized to 4; Added (Area=16.00, Perim=16.00, Square)
 *   Choice> 6 (Sorted View) ->
 *           ID 203: 4.00 x 4.00 | Area=16.00 | Perim=16.00 | Square
 *           ID 201: 5.00 x 4.00 | Area=20.00 | Perim=18.00 | Non-square
 *           ID 202: 6.50 x 3.50 | Area=22.75 | Perim=20.00 | Non-square
 *   Choice> 4 (Update) -> ID: 201 -> Width: 5.0, Height: 5.0 -> Updated (Area=25.00, Perim=20.00, Square)
 *   Choice> 8 (Summary) ->
 *           Largest Area : 25.00 (ID 201)
 *           Smallest Area: 16.00 (ID 203)
 *           Total Area   : 63.75 | Average Area: 21.25
 *   Choice> 0 (Exit) -> Freed rectangle array. Exiting.
 *
 * Edge Cases Tested:
 *   1. Invalid dimensions: Rejects 0, negative numbers, NaN, inf, and -inf.
 *   2. Duplicate ID: Rejected during add without mutating the inventory.
 *   3. Single rectangle: Largest and smallest area both report that single record.
 *   4. Equal areas: Ties in sorted view are broken by ascending rectangle ID.
 *   5. Square with non-integer dimensions (e.g., 3.14159 x 3.14159): Classified
 *      as Square using relative tolerance |w - h| <= 1e-5f * max(w, h).
 *   6. Empty list after deletion: Display, search, update, delete, sort, filter,
 *      and summary report clear messages when count == 0.
 *   7. Resizing failure / arithmetic overflow: Preserves original pointer safely.
 */

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_INIT_CAPACITY 1
#define MAX_INIT_CAPACITY 1000
#define LINE_BUF_SIZE 256
#define SQUARE_REL_TOL 1e-5f
#define AREA_EPSILON 1e-5f

typedef struct {
    int id;
    float width;
    float height;
    float area;
    float perimeter;
} Rectangle;

/* --- Core Geometry, Allocation, Assignment, Display & Cleanup Functions --- */

void calculate_area(Rectangle *r) {
    if (r == NULL) {
        return;
    }

    r->area = r->width * r->height;
}

void calculate_geometry(Rectangle *r) {
    if (r == NULL) {
        return;
    }

    calculate_area(r);
    r->perimeter = 2.0f * (r->width + r->height);
}

/*
 * Uses scaled tolerance |width - height| <= SQUARE_REL_TOL * max(width, height)
 * rather than exact floating-point equality.
 */
bool is_square(const Rectangle *r) {
    if (r == NULL) {
        return false;
    }

    float max_side = (r->width > r->height) ? r->width : r->height;
    float diff = fabsf(r->width - r->height);

    return diff <= (SQUARE_REL_TOL * max_side);
}

Rectangle *create_rectangle_array(int size) {
    if (size <= 0) {
        return NULL;
    }

    size_t count = (size_t)size;
    if (count > SIZE_MAX / sizeof(Rectangle)) {
        return NULL;
    }

    Rectangle *rectangles = malloc(count * sizeof(Rectangle));

    return rectangles;
}

bool resize_rectangle_array(Rectangle **rectangles, int *capacity) {
    if (rectangles == NULL || capacity == NULL || *capacity <= 0) {
        return false;
    }

    if (*capacity > INT_MAX / 2) {
        return false;
    }

    int new_capacity = (*capacity) * 2;
    size_t new_count = (size_t)new_capacity;
    if (new_count > SIZE_MAX / sizeof(Rectangle)) {
        return false;
    }

    Rectangle *resized = realloc(*rectangles, new_count * sizeof(Rectangle));
    if (resized == NULL) {
        return false;
    }

    *rectangles = resized;
    *capacity = new_capacity;
    printf("Notice: Capacity dynamically expanded to %d rectangles.\n", *capacity);

    return true;
}

void add_rectangle(Rectangle *rectangles, int index, int id, float width, float height) {
    if (rectangles == NULL || index < 0) {
        return;
    }

    rectangles[index].id = id;
    rectangles[index].width = width;
    rectangles[index].height = height;
    calculate_geometry(&rectangles[index]);
}

void display_rectangles(const Rectangle *rectangles, int size) {
    if (rectangles == NULL || size <= 0) {
        printf("Notice: No rectangles to display.\n");
        return;
    }

    printf("\n%-8s | %-10s | %-10s | %-12s | %-12s | %-12s\n",
           "ID", "Width", "Height", "Area", "Perimeter", "Classification");
    printf("---------+------------+------------+--------------+--------------+----------------\n");

    for (int i = 0; i < size; i++) {
        printf("%-8d | %-10.2f | %-10.2f | %-12.2f | %-12.2f | %-12s\n",
               rectangles[i].id,
               (double)rectangles[i].width,
               (double)rectangles[i].height,
               (double)rectangles[i].area,
               (double)rectangles[i].perimeter,
               is_square(&rectangles[i]) ? "Square" : "Non-square");
    }
    printf("\n");
}

void free_rectangle_array(Rectangle *rectangles) {
    free(rectangles);
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

bool read_finite_float(const char *prompt, bool strictly_positive, float *out_val) {
    char line[LINE_BUF_SIZE];
    bool is_overlong = false;

    printf("%s", prompt);
    if (!read_trimmed_line(line, sizeof(line), &is_overlong)) {
        return false;
    }

    if (is_overlong || line[0] == '\0') {
        printf("Error: Invalid floating-point input.\n");
        return false;
    }

    char *end_ptr = NULL;
    errno = 0;
    float parsed = strtof(line, &end_ptr);

    while (*end_ptr == ' ' || *end_ptr == '\t') {
        end_ptr++;
    }

    if (errno == ERANGE || *end_ptr != '\0' || !isfinite(parsed)) {
        printf("Error: Value must be a finite real number (NaN/Inf rejected).\n");
        return false;
    }

    if (strictly_positive && parsed <= 0.0f) {
        printf("Error: Dimension must be strictly positive (> 0).\n");
        return false;
    }

    if (!strictly_positive && parsed < 0.0f) {
        printf("Error: Threshold cannot be negative.\n");
        return false;
    }

    *out_val = parsed;

    return true;
}

/* --- Inventory Management Operations --- */

int find_rectangle_index(const Rectangle *rectangles, int size, int id) {
    if (rectangles == NULL || size <= 0) {
        return -1;
    }

    for (int i = 0; i < size; i++) {
        if (rectangles[i].id == id) {
            return i;
        }
    }

    return -1;
}

bool handle_add_rectangle(Rectangle **rectangles, int *count, int *capacity) {
    if (rectangles == NULL || count == NULL || capacity == NULL) {
        return false;
    }

    int id = 0;
    if (!read_int_range("Enter unique positive rectangle ID: ", 1, INT_MAX, &id)) {
        return false;
    }

    if (find_rectangle_index(*rectangles, *count, id) != -1) {
        printf("Error: Rectangle ID %d already exists.\n", id);
        return false;
    }

    float width = 0.0f;
    if (!read_finite_float("Enter positive width: ", true, &width)) {
        return false;
    }

    float height = 0.0f;
    if (!read_finite_float("Enter positive height: ", true, &height)) {
        return false;
    }

    if (!isfinite(width * height) || !isfinite(2.0f * (width + height))) {
        printf("Error: Dimensions overflow finite area/perimeter representation.\n");
        return false;
    }

    if (*count >= *capacity) {
        if (!resize_rectangle_array(rectangles, capacity)) {
            printf("Error: Memory reallocation failed; existing rectangles preserved.\n");
            return false;
        }
    }

    add_rectangle(*rectangles, *count, id, width, height);
    const Rectangle *added = &(*rectangles)[*count];
    (*count)++;

    printf("Success: Added Rectangle ID %d (Area: %.2f, Perimeter: %.2f, %s). [Count: %d/%d]\n",
           added->id,
           (double)added->area,
           (double)added->perimeter,
           is_square(added) ? "Square" : "Non-square",
           *count,
           *capacity);

    return true;
}

bool handle_search_rectangle(const Rectangle *rectangles, int count) {
    if (count <= 0) {
        printf("Notice: Rectangle inventory is empty.\n");
        return false;
    }

    int id = 0;
    if (!read_int_range("Enter rectangle ID to search: ", 1, INT_MAX, &id)) {
        return false;
    }

    int idx = find_rectangle_index(rectangles, count, id);
    if (idx < 0) {
        printf("Error: Rectangle with ID %d not found.\n", id);
        return false;
    }

    const Rectangle *r = &rectangles[idx];
    printf("Found -> ID: %d, Width: %.2f, Height: %.2f, Area: %.2f, Perimeter: %.2f, Type: %s\n",
           r->id,
           (double)r->width,
           (double)r->height,
           (double)r->area,
           (double)r->perimeter,
           is_square(r) ? "Square" : "Non-square");

    return true;
}

bool handle_update_rectangle(Rectangle *rectangles, int count) {
    if (count <= 0) {
        printf("Notice: Rectangle inventory is empty.\n");
        return false;
    }

    int id = 0;
    if (!read_int_range("Enter rectangle ID to update: ", 1, INT_MAX, &id)) {
        return false;
    }

    int idx = find_rectangle_index(rectangles, count, id);
    if (idx < 0) {
        printf("Error: Rectangle with ID %d not found.\n", id);
        return false;
    }

    float new_width = 0.0f;
    if (!read_finite_float("Enter new positive width: ", true, &new_width)) {
        return false;
    }

    float new_height = 0.0f;
    if (!read_finite_float("Enter new positive height: ", true, &new_height)) {
        return false;
    }

    if (!isfinite(new_width * new_height) || !isfinite(2.0f * (new_width + new_height))) {
        printf("Error: Dimensions overflow finite area/perimeter representation.\n");
        return false;
    }

    add_rectangle(rectangles, idx, id, new_width, new_height);
    const Rectangle *updated = &rectangles[idx];

    printf("Success: Updated Rectangle ID %d (Area: %.2f, Perimeter: %.2f, %s).\n",
           updated->id,
           (double)updated->area,
           (double)updated->perimeter,
           is_square(updated) ? "Square" : "Non-square");

    return true;
}

bool handle_delete_rectangle(Rectangle *rectangles, int *count) {
    if (count == NULL || *count <= 0) {
        printf("Notice: Rectangle inventory is empty.\n");
        return false;
    }

    int id = 0;
    if (!read_int_range("Enter rectangle ID to delete: ", 1, INT_MAX, &id)) {
        return false;
    }

    int idx = find_rectangle_index(rectangles, *count, id);
    if (idx < 0) {
        printf("Error: Rectangle with ID %d not found.\n", id);
        return false;
    }

    for (int i = idx; i < (*count) - 1; i++) {
        rectangles[i] = rectangles[i + 1];
    }

    (*count)--;
    printf("Success: Deleted Rectangle ID %d. [Remaining: %d]\n", id, *count);

    return true;
}

/* --- Sorting, Filtering & Summary Analytics --- */

int compare_rectangles_area_asc(const void *lhs, const void *rhs) {
    const Rectangle *a = (const Rectangle *)lhs;
    const Rectangle *b = (const Rectangle *)rhs;

    float diff = a->area - b->area;
    if (diff < -AREA_EPSILON) {
        return -1;
    }
    if (diff > AREA_EPSILON) {
        return 1;
    }

    if (a->id < b->id) {
        return -1;
    }
    if (a->id > b->id) {
        return 1;
    }

    return 0;
}

bool display_sorted_rectangles(const Rectangle *rectangles, int count) {
    if (rectangles == NULL || count <= 0) {
        printf("Notice: Rectangle inventory is empty; nothing to sort.\n");
        return false;
    }

    Rectangle *copy = create_rectangle_array(count);
    if (copy == NULL) {
        printf("Error: Failed to allocate memory for sorted display.\n");
        return false;
    }

    memcpy(copy, rectangles, (size_t)count * sizeof(Rectangle));
    qsort(copy, (size_t)count, sizeof(Rectangle), compare_rectangles_area_asc);

    printf("\nRectangles Sorted by Area (Ascending), Ties Broken by ID (Ascending):");
    display_rectangles(copy, count);

    free_rectangle_array(copy);

    return true;
}

bool filter_rectangles_by_area(const Rectangle *rectangles, int count) {
    if (rectangles == NULL || count <= 0) {
        printf("Notice: Rectangle inventory is empty; nothing to filter.\n");
        return false;
    }

    float threshold = 0.0f;
    if (!read_finite_float("Enter minimum area threshold (exclusive): ", false, &threshold)) {
        return false;
    }

    int matches = 0;
    for (int i = 0; i < count; i++) {
        if (rectangles[i].area > threshold) {
            if (matches == 0) {
                printf("\nRectangles with Area > %.2f:\n", (double)threshold);
                printf("%-8s | %-10s | %-10s | %-12s | %-12s | %-12s\n",
                       "ID", "Width", "Height", "Area", "Perimeter", "Classification");
                printf("---------+------------+------------+--------------+--------------+----------------\n");
            }

            printf("%-8d | %-10.2f | %-10.2f | %-12.2f | %-12.2f | %-12s\n",
                   rectangles[i].id,
                   (double)rectangles[i].width,
                   (double)rectangles[i].height,
                   (double)rectangles[i].area,
                   (double)rectangles[i].perimeter,
                   is_square(&rectangles[i]) ? "Square" : "Non-square");
            matches++;
        }
    }

    if (matches == 0) {
        printf("Notice: No rectangles found with area exceeding %.2f.\n", (double)threshold);
    } else {
        printf("Matching rectangles: %d\n\n", matches);
    }

    return true;
}

bool display_geometry_summary(const Rectangle *rectangles, int count) {
    if (rectangles == NULL || count <= 0) {
        printf("Notice: Rectangle inventory is empty; no area summary available.\n");
        return false;
    }

    int max_idx = 0;
    int min_idx = 0;
    double total_area = 0.0;

    for (int i = 0; i < count; i++) {
        total_area += (double)rectangles[i].area;

        if (rectangles[i].area > rectangles[max_idx].area) {
            max_idx = i;
        }
        if (rectangles[i].area < rectangles[min_idx].area) {
            min_idx = i;
        }
    }

    double average_area = total_area / (double)count;

    printf("\n--- Geometry Area Summary (%d Rectangles) ---\n", count);
    printf("Largest Area  : %.2f (ID %d: %.2f x %.2f)\n",
           (double)rectangles[max_idx].area,
           rectangles[max_idx].id,
           (double)rectangles[max_idx].width,
           (double)rectangles[max_idx].height);
    printf("Smallest Area : %.2f (ID %d: %.2f x %.2f)\n",
           (double)rectangles[min_idx].area,
           rectangles[min_idx].id,
           (double)rectangles[min_idx].width,
           (double)rectangles[min_idx].height);
    printf("Total Area    : %.2f\n", total_area);
    printf("Average Area  : %.2f\n\n", average_area);

    return true;
}

void print_menu(void) {
    printf("=== Rectangle Inventory & Geometry Analysis ===\n");
    printf("1. Add Rectangle\n");
    printf("2. Display All Rectangles\n");
    printf("3. Search Rectangle by ID\n");
    printf("4. Update Dimensions by ID\n");
    printf("5. Delete Rectangle by ID\n");
    printf("6. Display Rectangles Sorted by Area\n");
    printf("7. Filter Rectangles by Area Threshold\n");
    printf("8. Display Geometry Area Summary\n");
    printf("0. Exit\n");
}

int main(void) {
    int capacity = 0;
    while (true) {
        if (feof(stdin)) {
            printf("Input closed before initialization. Exiting.\n");
            return 1;
        }

        if (read_int_range("Enter initial rectangle capacity (1 - 1000): ",
                           MIN_INIT_CAPACITY, MAX_INIT_CAPACITY, &capacity)) {
            break;
        }
    }

    Rectangle *rectangles = create_rectangle_array(capacity);
    if (rectangles == NULL) {
        printf("Fatal Error: Memory allocation failed for capacity %d.\n", capacity);
        return 1;
    }

    int count = 0;
    printf("Initialized rectangle array with capacity %d.\n\n", capacity);

    bool running = true;
    while (running) {
        if (feof(stdin)) {
            printf("\nEnd of input reached. Releasing memory and exiting.\n");
            break;
        }

        print_menu();
        int choice = -1;
        if (!read_int_range("Select an option (0 - 8): ", 0, 8, &choice)) {
            continue;
        }

        switch (choice) {
            case 1: {
                handle_add_rectangle(&rectangles, &count, &capacity);
                break;
            }
            case 2: {
                printf("\nStored Rectangles (%d / %d capacity):", count, capacity);
                display_rectangles(rectangles, count);
                break;
            }
            case 3: {
                handle_search_rectangle(rectangles, count);
                break;
            }
            case 4: {
                handle_update_rectangle(rectangles, count);
                break;
            }
            case 5: {
                handle_delete_rectangle(rectangles, &count);
                break;
            }
            case 6: {
                display_sorted_rectangles(rectangles, count);
                break;
            }
            case 7: {
                filter_rectangles_by_area(rectangles, count);
                break;
            }
            case 8: {
                display_geometry_summary(rectangles, count);
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

    free_rectangle_array(rectangles);
    printf("Memory released. Program terminated cleanly.\n");

    return 0;
}
