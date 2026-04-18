#ifndef LOG_H
#define LOG_H

//append one line to <district> in the format: time-user-role-action
int log_action(const char *district, const char *user,
               const char *role, const char *action);

#endif 