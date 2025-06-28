#ifndef CHATRN_H
#define CHATRN_H

void *server_listen(void *);
void connect_to_request(const char *username);
void start_chat_session(int sock, const char *username);

#endif