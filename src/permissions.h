#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/stat.h>

//permission bits
#define PERM_DISTRICT_DIR   0750
#define PERM_REPORTS_DAT    0664
#define PERM_DISTRICT_CFG   0640
#define PERM_LOGGED         0644

#define ROLE_INSPECTOR  "inspector"
#define ROLE_MANAGER    "manager"

//chmod
int  perm_set(const char *path, mode_t mode);

//check if has read or write access on path
int  perm_check(const char *path, const char *role, const char *action);

//convert bits to 9-char string (rw-rw-r-)
void perm_to_string(mode_t mode, char *buf);

int  perm_verify(const char *path, mode_t expected);

#endif 