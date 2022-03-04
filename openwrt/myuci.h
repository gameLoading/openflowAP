#define MAX_UCI_SIZE 500
#define MAX_UCI_LEN 50

int uci_command_set(char *str);
int uci_command_del(char *str);
int uci_command_commit();
int uci_command_get(char *str, char *buffer);
int uci_command_show(char *str, char* buf);
