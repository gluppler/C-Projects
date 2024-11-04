# KeywordSearch

`KeywordSearch` is a command-line utility written in C that recursively searches for keywords within files in a specified directory.

## Features
- Search for multiple keywords with case-sensitive or case-insensitive options
- Highlight matches in the output
- Save results to a specified output file
- Filter files by extension

## Usage

```bash
./keyword_search <directory> <keyword1> [keyword2 ...] [-i] [-highlight] [-o <output_file>] [-ext <file_extension>]


### Options
- `-i`: Perform a case-insensitive search.
- `-highlight`: Highlight the found keywords in the output.
- `-o <output_file>`: Specify a file to save the results. If not provided, results are printed to the console.
- `-ext <file_extension>`: Limit the search to files with the specified extension (default: `txt`).

### Example

```bash
./keyword_search ./my_directory keyword1 keyword2 -i -highlight -o results.txt -ext txt


---

### Summary

This more complete proof of concept includes:

- Robust error handling
- Enhanced functionality with command-line options
- Support for keyword highlighting and filtering by file extension
- Clear documentation and structure for easy understanding and maintenance


