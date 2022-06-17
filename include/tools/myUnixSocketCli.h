#include "myUnixSocket.h"

int send_unix_msg_onetime(unix_socket_file file, char* msg);
int recv_unix_msg(unix_socket_file file, char* buffer, int buf_size);