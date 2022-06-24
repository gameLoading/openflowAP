#define MAX_DEVICE_SIZE 10
#define MAX_DEVICE_NAME_LEN 20
#define MAX_SSID_LEN 20
#define MAX_JSON_SIZE 50000
#define MAX_UCI_LEN 200

/*Control Wireless Device*/
int InitialWirelessDevice(void);

/*Query information for Wireless Device */
int getAllWirelessDevice(char buf[][MAX_DEVICE_NAME_LEN]);
void getAllWirelessDeviceWithJSON(char *buf, int buf_len);
void getWirelessDeviceInfo(char *deviceName, char *buf, int buf_len);
void getAssocicatedHosts(char *deviceName, char buf[], int buf_len);
int getWirelessUcis(char *deviceName, char buf[], int buf_len);
void getFreqList(char *deviceName, char buf[], int buf_len);
void getTxpowerList(char *deviceName, char buf[], int buf_len);
void getCountryList(char *deviceName, char buf[], int buf_len);
void getScannedSSIDList(char *deviceName, char buf[], int buf_len);
void getFreqSurveyInfo(char *deviceName, char buf[], int buf_len);
void getSystemBoard(char *deviceName, char buf[], int buf_len);
void getSystemInfo(char *deviceName, char buf[], int buf_len);

/*Managing function for Wireless Device */
void setSSID(char *deviceName, char *ssid, int buf_len);
void setWirelessUcis(char *deviceName, char *buf);
void setTxPower(char *deviceName, int txpower);
void restartWifi(void);

/*For utils*/
int isAvailableSSID(char *ssid);
int getBaseUci(char *buffer, char deviceName[], int buf_len);
