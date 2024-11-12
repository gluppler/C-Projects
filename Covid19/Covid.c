#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 512

// Structure to hold hospital data for each state on each date
typedef struct {
    char *date;
    char *state;
    int beds;               // Renamed from total_beds
    int beds_covid;
    int beds_noncritical;
    int admitted_pui;       // suspected
    int admitted_covid;
    int admitted_total;
} HospitalData;

// Function prototypes
void load_data(const char *filename, HospitalData **data, int *size);
void find_highest_bed_state(const HospitalData *data, int size, const char *date);
void calculate_bed_ratio(const HospitalData *data, int size, const char *date);
void average_category(const char *category, const HospitalData *data, int size, const char *date);
int compare_strings_case_insensitive(const char *a, const char *b);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <filename> <option> [category] [date]\n", argv[0]);
        printf("Options:\n");
        printf("  --highest-bed-state       Find the state with the highest hospital beds\n");
        printf("  --bed-ratio               Calculate the ratio of COVID-19 dedicated beds to total hospital beds\n");
        printf("  --average-category <x>    Calculate average admissions for a specified category (suspected/covid/total)\n");
        printf("Note: [category] argument is required for --average-category.\n");
        return 1;
    }

    const char *filename = argv[1];
    HospitalData *data = NULL;
    int size = 0;

    load_data(filename, &data, &size);

    const char *date = (argc > 4) ? argv[4] : NULL;

    if (strcmp(argv[2], "--highest-bed-state") == 0) {
        find_highest_bed_state(data, size, date);
    } else if (strcmp(argv[2], "--bed-ratio") == 0) {
        calculate_bed_ratio(data, size, date);
    } else if (strcmp(argv[2], "--average-category") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: Please specify a category.\n");
            return 1;
        }
        average_category(argv[3], data, size, date);
    } else {
        fprintf(stderr, "Error: Invalid option.\n");
        return 1;
    }

    // Free dynamically allocated memory
    for (int i = 0; i < size; i++) {
        free(data[i].date);
        free(data[i].state);
    }
    free(data);

    return 0;
}

void load_data(const char *filename, HospitalData **data, int *size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    fgets(line, MAX_LINE_LENGTH, file); // Skip header

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        *data = realloc(*data, (*size + 1) * sizeof(HospitalData));

        HospitalData *entry = &(*data)[*size];
        entry->date = malloc(MAX_LINE_LENGTH);
        entry->state = malloc(MAX_LINE_LENGTH);

        // Parse the CSV line
        if (sscanf(line, "%10[^,],%99[^,],%d,%d,%d,%d,%d,%d",
                   entry->date,
                   entry->state,
                   &entry->beds,            // Renamed from total_beds
                   &entry->beds_covid,
                   &entry->beds_noncritical,
                   &entry->admitted_pui,
                   &entry->admitted_covid,
                   &entry->admitted_total) != 8) {
            fprintf(stderr, "Error parsing line: %s\n", line);
            continue; // Skip the malformed line
        }

        (*size)++;
    }

    fclose(file);
}

void find_highest_bed_state(const HospitalData *data, int size, const char *date) {
    int max_beds = 0, max_covid_beds = 0, max_noncritical_beds = 0;
    const char *max_state_beds = NULL, *max_state_covid = NULL, *max_state_noncritical = NULL;

    for (int i = 0; i < size; i++) {
        if (date && !compare_strings_case_insensitive(data[i].date, date)) {
            continue;  // Skip entries not matching the specified date
        }

        if (data[i].beds > max_beds) {  // Changed from total_beds to beds
            max_beds = data[i].beds;
            max_state_beds = data[i].state;
        }

        if (data[i].beds_covid > max_covid_beds) {
            max_covid_beds = data[i].beds_covid;
            max_state_covid = data[i].state;
        }

        if (data[i].beds_noncritical > max_noncritical_beds) {
            max_noncritical_beds = data[i].beds_noncritical;
            max_state_noncritical = data[i].state;
        }
    }

    if (max_state_beds) {
        printf("State with the highest total beds: %s (%d beds)\n", max_state_beds, max_beds);
    }

    if (max_state_covid) {
        printf("State with the highest COVID-19 beds: %s (%d beds)\n", max_state_covid, max_covid_beds);
    }

    if (max_state_noncritical) {
        printf("State with the highest non-critical beds: %s (%d beds)\n", max_state_noncritical, max_noncritical_beds);
    }
}

void calculate_bed_ratio(const HospitalData *data, int size, const char *date) {
    int total_beds = 0, total_covid_beds = 0;

    for (int i = 0; i < size; i++) {
        if (date && !compare_strings_case_insensitive(data[i].date, date)) {
            continue;  // Skip entries not matching the specified date
        }
        total_beds += data[i].beds;            // Changed from total_beds to beds
        total_covid_beds += data[i].beds_covid;
    }

    if (total_beds > 0) {
        double ratio = (double)total_covid_beds / total_beds;
        printf("Ratio of COVID-19 dedicated beds to total hospital beds: %.2f\n", ratio);
    } else {
        printf("No data found for the specified date.\n");
    }
}

void average_category(const char *category, const HospitalData *data, int size, const char *date) {
    int total = 0, count = 0;

    for (int i = 0; i < size; i++) {
        if (date && !compare_strings_case_insensitive(data[i].date, date)) continue;

        int category_value = 0;
        if (strcmp(category, "suspected") == 0) {
            category_value = data[i].admitted_pui;  // Use admitted_pui but keep the "suspected" category
        } else if (strcmp(category, "covid") == 0) {
            category_value = data[i].admitted_covid;
        } else if (strcmp(category, "total") == 0) {
            category_value = data[i].admitted_total;
        } else {
            fprintf(stderr, "Invalid category. Choose from suspected, covid, or total.\n");
            exit(EXIT_FAILURE);
        }

        total += category_value;
        count++;
    }

    if (count > 0) {
        double average = (double)total / count;
        printf("Average %s admissions: %.2f\n", category, average);
    } else {
        printf("No data found for the specified date or category.\n");
    }
}

int compare_strings_case_insensitive(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

