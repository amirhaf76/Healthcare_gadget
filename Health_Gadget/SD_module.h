#ifndef SD_MODULE_H
#define SD_MODULE_H

bool print_in_file(int val, char * fileName);
bool print_in_file(double valn, char * fileName);
bool print_in_file(String val, char * fileName);

bool create_csv_file(const int buff[], size_t siz, char * fileName);

bool read_from_file(char * fileName);

#endif