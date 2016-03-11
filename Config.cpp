// 
// 
// 

#define DBG_OUTPUT_PORT Serial

#include "Config.h"
#include "FSWebServer.h"
#include "DynamicData.h"


strConfig config;
strApConfig apConfig;


// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c)
{
	if (c >= '0' && c <= '9') {
		return((unsigned char)c - '0');
	}
	if (c >= 'a' && c <= 'f') {
		return((unsigned char)c - 'a' + 10);
	}
	if (c >= 'A' && c <= 'F') {
		return((unsigned char)c - 'A' + 10);
	}
	return(0);
}

void defaultConfig (){
	// DEFAULT CONFIG
	config.ssid = "YOUR_WIFI_SSID";
	config.password = "YOUR_WIFI_PASSWD";
	config.dhcp = true;
	config.IP[0] = 192; config.IP[1] = 168; config.IP[2] = 5; config.IP[3] = 14;
	config.Netmask[0] = 255; config.Netmask[1] = 255; config.Netmask[2] = 255; config.Netmask[3] = 0;
	config.Gateway[0] = 192; config.Gateway[1] = 168; config.Gateway[2] = 5; config.Gateway[3] = 1;
	config.DNS[0] = 192; config.DNS[1] = 168; config.DNS[2] = 5; config.DNS[3] = 1;
	config.ntpServerName = "es.pool.ntp.org";
	config.Update_Time_Via_NTP_Every = 0;
	config.timezone = 10;
	config.daylight = true;
	config.DeviceName = "ESP8266fs";
	save_config();
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
}

boolean load_config() {
	File configFile = SPIFFS.open("/config.json", "r");
	if (!configFile) {
		Serial.println("Failed to open config file");
		return false;
	}

	size_t size = configFile.size();
	if (size > 660) {
		Serial.println("Config file size is too large");
		return false;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);
	configFile.close();
#ifdef DEBUG
	Serial.print("JSON file size: "); Serial.print(size); Serial.println(" bytes");
#endif

	StaticJsonBuffer<512> jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		Serial.println("Failed to parse config file");
		return false;
	}
#ifdef DEBUG
	String temp;
	json.prettyPrintTo(temp);
	Serial.println(temp);
#endif
	//memset(config.ssid, 0, 28);
	//memset(config.pass, 0, 50);
	//String("Virus_Detected!!!").toCharArray(config.ssid, 28); // Assign WiFi SSID
	//String("LaJunglaSigloXX1@.").toCharArray(config.pass, 50); // Assign WiFi PASS

	config.ssid = json["ssid"].asString();
	//String(ssid_str).toCharArray(config.ssid, 28);

	config.password = json["pass"].asString();
	
	config.IP[0] = json["ip"][0];
	config.IP[1] = json["ip"][1];
	config.IP[2] = json["ip"][2];
	config.IP[3] = json["ip"][3];

	config.Netmask[0] = json["netmask"][0];
	config.Netmask[1] = json["netmask"][1];
	config.Netmask[2] = json["netmask"][2];
	config.Netmask[3] = json["netmask"][3];

	config.Gateway[0] = json["gateway"][0];
	config.Gateway[1] = json["gateway"][1];
	config.Gateway[2] = json["gateway"][2];
	config.Gateway[3] = json["gateway"][3];

	config.DNS[0] = json["dns"][0];
	config.DNS[1] = json["dns"][1];
	config.DNS[2] = json["dns"][2];
	config.DNS[3] = json["dns"][3];

	config.dhcp = json["dhcp"];

	//String(pass_str).toCharArray(config.pass, 28);
	config.ntpServerName = json["ntp"].asString();
	config.Update_Time_Via_NTP_Every = json["NTPperiod"];
	config.timezone = json["timeZone"];
	config.daylight = json["daylight"];
	config.DeviceName = json["deviceName"].asString();

#ifdef DEBUG
	DBG_OUTPUT_PORT.println("Data initialized.");
	DBG_OUTPUT_PORT.print("SSID: "); Serial.println(config.ssid);
	DBG_OUTPUT_PORT.print("PASS: "); Serial.println(config.password);
	DBG_OUTPUT_PORT.print("NTP Server: "); Serial.println(config.ntpServerName);
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG
	return true;
}

boolean save_config() {
	//flag_config = false;
#ifdef DEBUG
	Serial.println("Save config");
#endif
	StaticJsonBuffer<660> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["ssid"] = config.ssid;
	json["pass"] = config.password;
	
	JsonArray& jsonip = json.createNestedArray("ip");
	jsonip.add(config.IP[0]);
	jsonip.add(config.IP[1]);
	jsonip.add(config.IP[2]);
	jsonip.add(config.IP[3]);
	
	JsonArray& jsonNM = json.createNestedArray("netmask");
	jsonNM.add(config.Netmask[0]);
	jsonNM.add(config.Netmask[1]);
	jsonNM.add(config.Netmask[2]);
	jsonNM.add(config.Netmask[3]);
	
	JsonArray& jsonGateway = json.createNestedArray("gateway");
	jsonGateway.add(config.Gateway[0]);
	jsonGateway.add(config.Gateway[1]);
	jsonGateway.add(config.Gateway[2]);
	jsonGateway.add(config.Gateway[3]);
	
	JsonArray& jsondns = json.createNestedArray("dns");
	jsondns.add(config.DNS[0]);
	jsondns.add(config.DNS[1]);
	jsondns.add(config.DNS[2]);
	jsondns.add(config.DNS[3]);
	
	json["dhcp"] = config.dhcp;
	json["ntp"] = config.ntpServerName;
	json["NTPperiod"] = config.Update_Time_Via_NTP_Every;
	json["timeZone"] = config.timezone;
	json["daylight"] = config.daylight;
	json["deviceName"] = config.DeviceName;
			
	//TODO add AP data to html
	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
		Serial.println("Failed to open config file for writing");
		return false;
	}

#ifdef DEBUG
	String temp;
	json.prettyPrintTo(temp);
	Serial.println(temp);
#endif

	json.printTo(configFile);
	configFile.flush();
	configFile.close();
	return true;
}

