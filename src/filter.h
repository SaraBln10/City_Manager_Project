#ifndef FILTER_H
#define FILTER_H

#include "report.h"

//splits in 3 parts
int parse_condition(const char *input, char *field, char *op, char *value);

int match_condition(const Report *r, const char *field,
                    const char *op, const char *value);

//display all records from district mathing the condition
int filter_reports(const char *district, const char **conditions, int n_conditions);

#endif