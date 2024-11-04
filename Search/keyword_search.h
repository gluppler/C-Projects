#ifndef KEYWORD_SEARCH_H
#define KEYWORD_SEARCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>

// Define maximum limits
#define MAX_LINE_LENGTH 1024
#define MAX_KEYWORDS 10

// ANSI color codes for highlighting matches
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

// Function Prototypes

/**
 * @brief Recursively searches for files in a directory and matches keywords.
 *
 * @param dir_name Directory to start searching from.
 * @param keywords Array of keywords to search for.
 * @param keyword_count Number of keywords provided.
 * @param case_sensitive Perform case-sensitive search if non-zero.
 * @param highlight Highlight found keywords in output if non-zero.
 * @param output_file File to save results to; NULL for console output.
 * @param file_ext Only searches files with this extension.
 */
void search_directory(const char *dir_name, const char *keywords[], int keyword_count, int case_sensitive, int highlight, FILE *output_file, const char *file_ext);

/**
 * @brief Searches a file for specified keywords and outputs matches.
 *
 * @param file_path Path of the file to search.
 * @param keywords Array of keywords to search for.
 * @param keyword_count Number of keywords provided.
 * @param case_sensitive Perform case-sensitive search if non-zero.
 * @param highlight Highlight found keywords in output if non-zero.
 * @param output_file File to save results to; NULL for console output.
 */
void search_file(const char *file_path, const char *keywords[], int keyword_count, int case_sensitive, int highlight, FILE *output_file);

/**
 * @brief Performs a case-insensitive search for a keyword within a string.
 *
 * @param str The string to search within.
 * @param keyword The keyword to search for.
 * @return Non-zero if the keyword is found; otherwise, zero.
 */
int case_insensitive_search(const char *str, const char *keyword);

/**
 * @brief Checks if a file has a specified extension.
 *
 * @param file_path The file path to check.
 * @param extension The file extension to match.
 * @return Non-zero if the extension matches; otherwise, zero.
 */
int has_file_extension(const char *file_path, const char *extension);

#endif // KEYWORD_SEARCH_H

