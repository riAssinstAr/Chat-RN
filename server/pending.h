#ifndef PENDING_H
#define PENDING_H

void pending_list(void);
void pending_add(const char *username, int sock);
int pending_get(const char *username);

#endif