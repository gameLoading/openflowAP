#include "wireless/wdev.h"
#include "wireless/myubus.h"
#include "wireless/myuci.h"
#include <string.h>
#include <syslog.h>
#include <json-c/json.h>

void InitialWirelessDevice()
{
	char buf[MAX_UCI_LEN];

	int res = 1;
	int ret = 0;
	char queryDev[MAX_UCI_LEN];
	// char tmp[MAX_UCI_LEN];

	/*All wifi device delete*/
	for (int i = 0; ret == 0; i++)
	{
		sprintf(queryDev, "wireless.wifinet%d", i);
		ret = uci_command_get(queryDev, buf);
		// sprintf(queryDev, "wireless.wifinet%d", i);
		// printf("uci get %s result code : %d\n", queryDev, ret);
		if (ret == 0)
		{
			sprintf(queryDev, "wireless.wifinet%d", i);
			ret = uci_command_del(queryDev);
			// sprintf(queryDev, "wireless.wifinet%d", i);
			// printf("uci del %s result code : %d\n", queryDev, ret);
		}
	}

	char command[5][MAX_UCI_LEN] = {
		"wireless.wifinet0=wifi-iface",
		"wireless.wifinet0.device=radio0",
		"wireless.wifinet0.mode=ap",
		"wireless.wifinet0.ssid=OpenWrt-public",
		"wireless.wifinet0.encryption=none",
	};
	for (int i = 0; i < 5; i++)
	{
		ret &= uci_command_set(command[i]);
	}

	ret &= uci_command_commit();
	printf("uci commit result code : %d\n", res);

	if (ret == 0)
	{
		printf("Wireless Device Initial Complete\n");
		restartWifi();
	}
	else
	{
		// printf("Wireless Device Initial fail\n");
		printf("Wireless Device Adding result code : %d\n", ret);
	}
}

void setSSID(char *ssid)
{
	if (!isAvailableSSID(ssid))
	{
		openlog("ofproto.c", LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "this ssid is not available \n");
		closelog();
		return;
	}
	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, "this ssid is available \n");
	closelog();
	int ret = 0;
	char command[MAX_UCI_LEN];
	sprintf(command, "wireless.wifinet0.ssid=%s", ssid);
	ret &= uci_command_set(command);
	uci_command_commit();

	openlog("ofproto.c", LOG_CONS, LOG_USER);
	syslog(LOG_INFO, "restartWifi() called \n");
	closelog();
	restartWifi();
}

void getAllWirelessDevice(char buf[][MAX_DEVICE_NAME_LEN])
{
	char result[MAX_JSON_SIZE] = "";
	ubus_cmd_call("iwinfo", "devices", result, NULL);
	result[MAX_JSON_SIZE - 1] = '\0';
	// printf("%s---4", result);

	struct json_object *jobj;

	struct json_tokener *tok = json_tokener_new();
	jobj = json_tokener_parse_ex(tok, result, strlen(result));

	if (tok->err != json_tokener_success)
	{
		printf("%s\n", json_tokener_error_desc(tok->err));
	}

	struct json_object *devices_json, *device_json;
	devices_json = json_object_object_get(jobj, "devices");

	memset(buf, 0, sizeof(char) * MAX_DEVICE_SIZE * MAX_DEVICE_NAME_LEN);

	for (int i = 0; i < json_object_array_length(devices_json); i++)
	{
		device_json = json_object_array_get_idx(devices_json, i);
		strncpy(buf[i], json_object_get_string(device_json), MAX_DEVICE_NAME_LEN - 1);
		buf[i][MAX_DEVICE_NAME_LEN - 1] = '\0';
	}
	json_tokener_free(tok);
}

void getAllWirelessDeviceWithJSON(char *buf)
{
	ubus_cmd_call("iwinfo", "devices", buf, NULL);
}

void getWirelessDeviceInfo(char *deviceName, char *buf)
{
	char arg[MAX_DEVICE_NAME_LEN + 10];
	sprintf(arg, "\'{\"device\":\"%s\"}\'", deviceName);
	ubus_cmd_call("iwinfo", "info", buf, arg);
}

void getAssocicatedHosts(char *deviceName, char *buf)
{
	char arg[MAX_DEVICE_NAME_LEN + 10];
	sprintf(arg, "\'{\"device\":\"%s\"}\'", deviceName);
	ubus_cmd_call("iwinfo", "assoclist", buf, arg);
}

void restartWifi()
{
	ubus_cmd_just_call("network", "restart");
}

int isAvailableSSID(char *ssid)
{
	for (int i = 0; ssid[i] != '\0';i++)
	{
		if (!(isalnum(ssid[i]) || ssid[i] == '_' || ssid[i] == '.'))
			return 0;
	}
	return 1;
}
