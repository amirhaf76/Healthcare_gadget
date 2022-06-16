#ifndef SD_MODULE_H
#define SD_MODULE_H

bool make_file_ready();
bool print_in_file(uint8_t val);
bool create_csv_file(const int buff[], size_t siz);
bool read_from_file();

#endif