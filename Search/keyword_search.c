#include "keyword_search.h"

// Case-insensitive search implementation
int case_insensitive_search(const char *str, const char *keyword) {
    for (; *str; str++) {
        if (tolower(*str) == tolower(*keyword)) {
            const char *s = str, *k = keyword;
            while (*s && *k && tolower(*s) == tolower(*k)) {
                s++; k++;
            }
            if (!*k) return 1;
        }
    }
    return 0;
}

// Check if file has the specified extension
int has_file_extension(const char *file_path, const char *extension) {
    const char *dot = strrchr(file_path, '.');
    return dot && strcmp(dot + 1, extension) == 0;
}

// Recursive directory search
void search_directory(const char *dir_name, const char *keywords[], int keyword_count, int case_sensitive, int highlight, FILE *output_file, const char *file_ext) {
    DIR *dir = opendir(dir_name);
    if (!dir) {
        fprintf(stderr, "Error opening directory '%s': %s\n", dir_name, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
                char subdir_path[MAX_LINE_LENGTH];
                snprintf(subdir_path, sizeof(subdir_path), "%s/%s", dir_name, entry->d_name);
                search_directory(subdir_path, keywords, keyword_count, case_sensitive, highlight, output_file, file_ext);
            }
        } else if (has_file_extension(entry->d_name, file_ext)) {
            char file_path[MAX_LINE_LENGTH];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_name, entry->d_name);
            search_file(file_path, keywords, keyword_count, case_sensitive, highlight, output_file);
        }
    }
    closedir(dir);
}

void search_file(const char *file_path, const char *keywords[], int keyword_count, int case_sensitive, int highlight, FILE *output_file) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "Error opening file '%s': %s\n", file_path, strerror(errno));
        return;
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        for (int i = 0; i < keyword_count; i++) {
            int found = case_sensitive ? strstr(line, keywords[i]) != NULL : case_insensitive_search(line, keywords[i]);
            if (found) {
                FILE *out = output_file ? output_file : stdout;
                fprintf(out, "Match in %s [line %d]: ", file_path, line_number);

                if (highlight) {
                    const char *remaining_line = line;
                    const char *match;
                    while ((match = case_sensitive ? strstr(remaining_line, keywords[i]) : case_insensitive_search(remaining_line, keywords[i]))) {
                        // Print text up to the match
                        fwrite(remaining_line, 1, match - remaining_line, out);

                        // Print the match in red
                        fprintf(out, ANSI_COLOR_RED "%.*s" ANSI_COLOR_RESET, (int)strlen(keywords[i]), match);

                        // Move past the matched keyword
                        remaining_line = match + strlen(keywords[i]);
                    }
                    // Print the rest of the line after the last match
                    fprintf(out, "%s", remaining_line);
                } else {
                    // Print line without highlighting
                    fprintf(out, "%s", line);
                }
                break;
            }
        }
    }
    fclose(file);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <directory> <keyword1> [keyword2 ...] [-i] [-highlight] [-o <output_file>] [-ext <file_extension>]\n", argv[0]);
        return 1;
    }

    const char *directory = argv[1];
    const char *keywords[MAX_KEYWORDS];
    int keyword_count = 0;
    int case_sensitive = 1;
    int highlight = 0;
    FILE *output_file = NULL;
    const char *file_ext = "txt";  // Default file extension

    for (int i = 2; i < argc && keyword_count < MAX_KEYWORDS; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            case_sensitive = 0;
        } else if (strcmp(argv[i], "-highlight") == 0) {
            highlight = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = fopen(argv[++i], "w");
            if (!output_file) {
                fprintf(stderr, "Error opening output file '%s': %s\n", argv[i], strerror(errno));
                return 1;
            }
        } else if (strcmp(argv[i], "-ext") == 0 && i + 1 < argc) {
            file_ext = argv[++i];
        } else {
            keywords[keyword_count++] = argv[i];
        }
    }

    search_directory(directory, keywords, keyword_count, case_sensitive, highlight, output_file, file_ext);

    if (output_file) fclose(output_file);
    return 0;
}

