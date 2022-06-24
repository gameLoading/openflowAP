#include "wireless/wdev.h"
#include <string>
#include <vector>

using namespace std;

class wireless_controller{
private:
    string device_name;
    vector<int> txpower_list;
public:
    void init_controller(string device_name);
    int get_current_txpower();
    bool up_txpower();
    bool down_txpower();
    void set_txpower(int txpower);
    bool is_available_txpower(int txpower);
    void update_txpower_list();
    bool is_broadcom_chipset();
    void set_max_txpower();
};