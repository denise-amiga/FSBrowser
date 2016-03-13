// 
// 
// 

#define DBG_OUTPUT_PORT Serial

#include "global.h"
#include "Config.h"
#include <ESP8266WiFi.h>
#include "DynamicData.h"


int AdminTimeOutCounter = 0;

Ticker secondTk;
boolean secondFlag = false;
ntpClient* ntp;
//boolean wifiIsConnected = false;
long wifiDisconnectedSince = 0;
wifiStatus currentWifiStatus = FIRST_RUN;
WebSocketsServer wsServer = WebSocketsServer(81);



void ConfigureWifi()
{
	WiFi.mode(WIFI_STA);
	currentWifiStatus = WIFI_STA_DISCONNECTED;
#ifdef DEBUG
	DBG_OUTPUT_PORT.printf("Connecting to %s\n", config.ssid.c_str());
#endif // DEBUG
	WiFi.begin(config.ssid.c_str(), config.password.c_str());
	if (!config.dhcp)
	{
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("NO DHCP");
#endif // DEBUG
		WiFi.config(
			IPAddress(config.IP[0], config.IP[1], config.IP[2], config.IP[3]), 
			IPAddress(config.Gateway[0], config.Gateway[1], config.Gateway[2], config.Gateway[3]), 
			IPAddress(config.Netmask[0], config.Netmask[1], config.Netmask[2], config.Netmask[3]),
			IPAddress(config.DNS[0], config.DNS[1], config.DNS[2], config.DNS[3])
		);
	}
	delay(2000);


	while (!WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
	}
	if (WiFi.isConnected()) {
		currentWifiStatus = WIFI_STA_CONNECTED;
	}

#ifdef DEBUG
	DBG_OUTPUT_PORT.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
	DBG_OUTPUT_PORT.printf("Gateway:    %s\n", WiFi.gatewayIP().toString().c_str());
	DBG_OUTPUT_PORT.printf("DNS:        %s\n", WiFi.dnsIP().toString().c_str());
	Serial.println(__PRETTY_FUNCTION__);

#endif // DEBUG
}

void ConfigureWifiAP() {
#ifdef DEBUG
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG
	WiFi.disconnect();
	WiFi.mode(WIFI_AP);
	String APname = apConfig.APssid;
	APname += (String)ESP.getChipId();
	WiFi.softAP(APname.c_str(), apConfig.APpassword.c_str());
}

void secondTick()
{
	secondFlag = true;
	/*strDateTime tempDateTime;
	AdminTimeOutCounter++;
	cNTP_Update++;
	UnixTimestamp++;
	ConvertUnixTimeStamp(UnixTimestamp + (config.timezone * 360), &tempDateTime);
	if (config.daylight) // Sommerzeit beachten
		if (summertime(tempDateTime.year, tempDateTime.month, tempDateTime.day, tempDateTime.hour, 0))
		{
			ConvertUnixTimeStamp(UnixTimestamp + (config.timezone * 360) + 3600, &DateTime);
		}
		else
		{
			DateTime = tempDateTime;
		}
	else
	{
		DateTime = tempDateTime;
	}
	Refresh = true;*/
}

void secondTask() {
	//DBG_OUTPUT_PORT.println(ntp->getTimeString());
	sendTimeData();
}

String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
	char c;
	String ret = "";

	for (byte t = 0; t<input.length(); t++)
	{
		c = input[t];
		if (c == '+') c = ' ';
		if (c == '%') {


			t++;
			c = input[t];
			t++;
			c = (h2int(c) << 4) | h2int(input[t]);
		}

		ret.concat(c);
	}
	return ret;

}

//
// Check the Values is between 0-255
//
boolean checkRange(String Value)
{
	if (Value.toInt() < 0 || Value.toInt() > 255)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void WiFiEvent(WiFiEvent_t event) {
	/*String eventStr;
	switch (event) {
	case WIFI_EVENT_STAMODE_CONNECTED:
		eventStr = "STAMODE_CONNECTED"; break;
	case WIFI_EVENT_STAMODE_DISCONNECTED:
		eventStr = "STAMODE_DISCONNECTED"; break;
	case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
		eventStr = "STAMODE_AUTHMODE_CHANGE"; break;
	case WIFI_EVENT_STAMODE_GOT_IP:
		eventStr = "STAMODE_GOT_IP"; break;
	case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
		eventStr = "STAMODE_DHCP_TIMEOUT"; break;
	case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
		eventStr = "SOFTAPMODE_STACONNECTED"; break;
	case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
		eventStr = "SOFTAPMODE_STADISCONNECTED"; break;
	case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
		eventStr = "SOFTAPMODE_PROBEREQRECVED"; break;
	case WIFI_EVENT_MAX:
		eventStr = "MAX_EVENTS"; break;
	}
	DBG_OUTPUT_PORT.printf("%s: %s\n",__PRETTY_FUNCTION__,eventStr.c_str());
	DBG_OUTPUT_PORT.printf("Current WiFi status: %d\n", currentWifiStatus);*/
	switch (event) {
		case WIFI_EVENT_STAMODE_GOT_IP:
			//DBG_OUTPUT_PORT.println(event);
			digitalWrite(CONNECTION_LED, LOW); // Turn LED on
			//wifiIsConnected = true;
			wifiDisconnectedSince = 0;
			currentWifiStatus = WIFI_STA_CONNECTED;
			break;
		case WIFI_EVENT_STAMODE_DISCONNECTED:
#ifdef DEBUG
			DBG_OUTPUT_PORT.println("case STA_DISCONNECTED");
#endif // DEBUG
			digitalWrite(CONNECTION_LED, HIGH); // Turn LED off
			if (currentWifiStatus == WIFI_STA_CONNECTED) {
				currentWifiStatus == WIFI_STA_DISCONNECTED;
				//wifiIsConnected = false;
				wifiDisconnectedSince = millis();
			}
			DBG_OUTPUT_PORT.printf("Disconnected for %d seconds\n", (millis() - wifiDisconnectedSince)/1000);
			/*if (!config.APEnable) {
				DBG_OUTPUT_PORT.println("Disabling STA. Starting AP");
				if ((millis() - wifiDisconnectedSince) > 10000) {
					DBG_OUTPUT_PORT.println("AP mode started");
					ConfigureWifiAP();
					APMode = true;
				}
			}*/
	}
}