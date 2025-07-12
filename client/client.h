#ifndef CLIENT_H
#define CLIENT_H

void initiate_connection(const char *);
void accept_session(const char *user);
void decline_session(const char *user);

#endif