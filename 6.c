// Unions (Extended: Memory-Efficient Typed Records and Safe Union Access)
/*
 * Task Description:
 *   Demonstrates memory layout differences between StructData and UnionData
 *   using sizeof, _Alignof, and offsetof without performing unsafe stale reads
 *   on inactive union members. Implements a tagged-union record system
 *   (TypedRecord) pairing UnionData with enum ValueType, validated setters for
 *   int, float, and bounded strings (<= 19 chars + '\0'), tag-checked display,
 *   a five-record mixed-type demonstration, edge-case checks, and written
 *   memory observations.
 *
 * Sample Run:
 *   === 1. Memory Layout, Alignment & Offsets ===
 *   StructData: sizeof = 28 bytes, alignof = 4 bytes
 *     Offsets (offsetof): i = 0, f = 4, str = 8
 *   UnionData : sizeof = 20 bytes, alignof = 4 bytes
 *     Addresses: &udata.i == &udata.f == &udata.str
 *   100 StructData = 2800 bytes | 100 UnionData = 2000 bytes
 *   TypedRecord: sizeof = 24 bytes, alignof = 4 bytes
 *
 *   === 2. Five Mixed-Type Tagged Records ===
 *   Record 0 [INT]   : 10
 *   Record 1 [FLOAT] : 220.50
 *   Record 2 [TEXT]  : "Union Programming"
 *   Record 3 [INT]   : 2026
 *   Record 4 [TEXT]  : "Typed Union"
 *
 *   === 3. Edge-Case Verification ===
 *   - 19-char string accepted; 20-char overlength string rejected safely
 *   - Single record transitioned: INT -> TEXT -> FLOAT
 *   - Invalid tag (99) blocked from reading union memory
 *
 * Edge Cases Tested:
 *   1. Maximum-length valid string (19 characters + null terminator).
 *   2. Overlength string (>= 20 characters) rejected without buffer overflow.
 *   3. Switching a single record from VALUE_INT to VALUE_TEXT to VALUE_FLOAT.
 *   4. Invalid/corrupted enum ValueType tag handled without reading union bytes.
 *   5. Platform-independent sizeof, _Alignof, and offsetof inspection.
 */

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define STR_BUF_SIZE 20
#define MAX_STR_LEN (STR_BUF_SIZE - 1)
#define BATCH_RECORD_COUNT 100
#define MIXED_RECORD_COUNT 5

typedef union {
    int i;
    float f;
    char str[STR_BUF_SIZE];
} UnionData;

typedef struct {
    int i;
    float f;
    char str[STR_BUF_SIZE];
} StructData;

typedef enum {
    VALUE_UNSET = 0,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_TEXT
} ValueType;

typedef struct {
    ValueType tag;
    UnionData value;
} TypedRecord;

/* --- Memory Layout Analysis --- */

void inspect_memory_layout(void) {
    StructData sdata = {10, 220.5f, "Struct Programming"};
    UnionData udata;
    memset(&udata, 0, sizeof(udata));

    printf("=== 1. Memory Layout, Alignment & Offsets ===\n");
    printf("StructData -> sizeof: %zu bytes, alignof: %zu bytes\n",
           sizeof(StructData), alignof(StructData));
    printf("  Field offsets (offsetof): i = %zu, f = %zu, str = %zu\n",
           offsetof(StructData, i),
           offsetof(StructData, f),
           offsetof(StructData, str));
    printf("  Member addresses:\n");
    printf("    &sdata.i   = %p (value: %d)\n", (void *)&sdata.i, sdata.i);
    printf("    &sdata.f   = %p (value: %.2f)\n", (void *)&sdata.f, (double)sdata.f);
    printf("    &sdata.str = %p (value: \"%s\")\n\n", (void *)&sdata.str, sdata.str);

    printf("UnionData  -> sizeof: %zu bytes, alignof: %zu bytes\n",
           sizeof(UnionData), alignof(UnionData));
    printf("  Member addresses (all begin at the exact same base address):\n");
    printf("    &udata.i   = %p\n", (void *)&udata.i);
    printf("    &udata.f   = %p\n", (void *)&udata.f);
    printf("    &udata.str = %p\n\n", (void *)&udata.str);

    printf("Memory Comparison for %d Records (using sizeof):\n", BATCH_RECORD_COUNT);
    printf("  %d x StructData  = %zu bytes\n",
           BATCH_RECORD_COUNT, BATCH_RECORD_COUNT * sizeof(StructData));
    printf("  %d x UnionData   = %zu bytes\n",
           BATCH_RECORD_COUNT, BATCH_RECORD_COUNT * sizeof(UnionData));
    printf("  %d x TypedRecord = %zu bytes (sizeof(TypedRecord) = %zu, alignof = %zu)\n\n",
           BATCH_RECORD_COUNT,
           BATCH_RECORD_COUNT * sizeof(TypedRecord),
           sizeof(TypedRecord),
           alignof(TypedRecord));
}

/* --- Tagged Record Initialization & Validated Setters --- */

void init_record(TypedRecord *rec) {
    if (rec == NULL) {
        return;
    }

    memset(rec, 0, sizeof(*rec));
    rec->tag = VALUE_UNSET;
}

bool set_record_int(TypedRecord *rec, int val) {
    if (rec == NULL) {
        return false;
    }

    rec->value.i = val;
    rec->tag = VALUE_INT;

    return true;
}

bool set_record_float(TypedRecord *rec, float val) {
    if (rec == NULL) {
        return false;
    }

    rec->value.f = val;
    rec->tag = VALUE_FLOAT;

    return true;
}

bool set_record_text(TypedRecord *rec, const char *text) {
    if (rec == NULL || text == NULL) {
        return false;
    }

    size_t len = strlen(text);
    if (len > MAX_STR_LEN) {
        return false;
    }

    memcpy(rec->value.str, text, len + 1);
    rec->tag = VALUE_TEXT;

    return true;
}

/* --- Tag-Checked Safe Display --- */

bool display_record(const TypedRecord *rec, int record_num) {
    if (rec == NULL) {
        printf("Record %d: <NULL pointer>\n", record_num);
        return false;
    }

    switch (rec->tag) {
        case VALUE_UNSET: {
            printf("Record %d [UNSET] : <No active value>\n", record_num);
            return true;
        }
        case VALUE_INT: {
            printf("Record %d [INT]   : %d\n", record_num, rec->value.i);
            return true;
        }
        case VALUE_FLOAT: {
            printf("Record %d [FLOAT] : %.2f\n", record_num, (double)rec->value.f);
            return true;
        }
        case VALUE_TEXT: {
            printf("Record %d [TEXT]  : \"%s\"\n", record_num, rec->value.str);
            return true;
        }
        default: {
            printf("Record %d [ERROR] : Invalid tag (%d); stale/corrupt union read prevented.\n",
                   record_num, (int)rec->tag);
            return false;
        }
    }
}

/* --- Edge-Case & Transition Verification --- */

void run_edge_cases(void) {
    printf("=== 3. Edge-Case & State Transition Tests ===\n");

    TypedRecord temp;
    init_record(&temp);

    /* Transition: INT -> TEXT (max 19 chars) -> FLOAT */
    set_record_int(&temp, 42);
    printf("Set INT   -> ");
    display_record(&temp, 1);

    const char *valid_19 = "1234567890123456789";
    if (set_record_text(&temp, valid_19)) {
        printf("Set 19-char valid string -> ");
        display_record(&temp, 1);
    }

    const char *invalid_20 = "12345678901234567890";
    if (!set_record_text(&temp, invalid_20)) {
        printf("Rejected 20-char overlength string; preserved previous -> ");
        display_record(&temp, 1);
    }

    set_record_float(&temp, 99.95f);
    printf("Switched to FLOAT -> ");
    display_record(&temp, 1);

    /* Invalid tag check */
    TypedRecord bad_rec;
    init_record(&bad_rec);
    bad_rec.tag = (ValueType)99;
    display_record(&bad_rec, 2);
    printf("\n");
}

void print_observations(void) {
    printf("=== 4. Written Interpretation & Observations ===\n");
    printf("1. Offsets, Alignment & Padding:\n");
    printf("   In StructData, members are placed sequentially at offsets determined by\n");
    printf("   each field's alignment (e.g., int and float typically align on 4-byte\n");
    printf("   boundaries, while char[20] has 1-byte alignment). The compiler inserts\n");
    printf("   padding bytes whenever needed so every member and the overall struct\n");
    printf("   satisfy their alignment requirements.\n");
    printf("2. Union Memory Overlap vs. Coexistence:\n");
    printf("   In UnionData, all members start at offset 0 and share the same storage,\n");
    printf("   so sizeof(UnionData) only needs to hold the largest member (padded to\n");
    printf("   the union's strictest alignment). This saves substantial memory across\n");
    printf("   arrays when a record holds only ONE type at a time, at the trade-off\n");
    printf("   that members cannot coexist simultaneously and must be guarded by a tag.\n");
}

int main(void) {
    inspect_memory_layout();

    printf("=== 2. Mixed-Type Tagged Records Demonstration ===\n");
    TypedRecord records[MIXED_RECORD_COUNT];
    for (int idx = 0; idx < MIXED_RECORD_COUNT; idx++) {
        init_record(&records[idx]);
    }

    set_record_int(&records[0], 10);
    set_record_float(&records[1], 220.5f);
    set_record_text(&records[2], "Union Programming");
    set_record_int(&records[3], 2026);
    set_record_text(&records[4], "Safe Union Access");

    for (int idx = 0; idx < MIXED_RECORD_COUNT; idx++) {
        display_record(&records[idx], idx + 1);
    }
    printf("\n");

    run_edge_cases();
    print_observations();

    return 0;
}
