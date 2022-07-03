#ifndef SD_MODULE_H
#define SD_MODULE_H

bool print_in_file(int val);
bool print_in_file(double val);
bool print_in_file(String val);
bool create_csv_file(const int buff[], size_t siz);
bool read_from_file();

#endif