#ifndef DISTRICT_H
#define DISTRICT_H

#define REPORTS_FILE    "reports.dat"
#define CONFIG_FILE     "district.cfg"
#define LOG_FILE        "logged_district"
#define SYMLINK_PREFIX  "active_reports-"

#define DEFAULT_THRESHOLD 1

//checks correct permissions
int  district_init(const char *district);

char *district_path(const char *district, const char *filename,
                    char *buf, int len);

//severity threshold
int  district_get_threshold(const char *district);

//update
int  district_set_threshold(const char *district, int value);

int  district_update_symlink(const char *district);

#endif 