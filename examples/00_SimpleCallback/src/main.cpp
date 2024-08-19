#include "Arduino.h"
#include "cronos.hpp"
#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"


// We need WiFi connection to get time from ntp
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASS";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";

/**
 * A proper way to handle TimeZones with daylightOffset
 * would be to specify an environment variable with TimeZone definition including daylight adjustmnet rules.
 * A list of rules for your zone could be obtained from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
 */
const char *time_zone = "MSK-3";    // TimeZone rule for Europe/Moscow


// CronoS object
CronoS cron;



// this function prints local time
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
  // reevaluate rules on time change
  cron.reload();
}


// crontab rule to execute each 10 seconds
const char *cron_each_10sec = "*/10 * * * * *";

// callback function one
void task_10sec(cronos_tid id, void* arg){
    printLocalTime();
    Serial.printf("Run callback task_10sec as cronos job id:%u\n", id);
}

// crontab rule to execute on 1,8,12 second every 3rd minute
const char *cron_example = "1,8,16 */3 * * * *";

// callback function one
void task_example1(cronos_tid id, void* arg){
    printLocalTime();
    Serial.printf("Run callback task_example1 as cronos job id:%u\n", id);
}



void setup() {
    Serial.begin(115200);

    // First step is to configure WiFi STA and connect in order to get the current time and date.
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");

    // set ntp notification call-back function
    sntp_set_time_sync_notification_cb(timeavailable);

    /**
     * Set NTP and timezone
     */
    configTzTime(time_zone, ntpServer1, ntpServer2);


    // create cron task with a callback task_10sec
    cron.addCallback(cron_each_10sec, task_10sec);

    // create cron task with a callback task_example1
    cron.addCallback(cron_example, task_example1);

    // start Cron scheduler
    cron.start();
/*
    A tasks will be executed asynchronously triggered via RTOS timer daemon task, more details on it's operation is
    available in ESP-IDF documentation
    https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html?highlight=timerhandle_t#timer-api

*/
}



void loop() {
    // nothing to do here
    delay(1000);
}
