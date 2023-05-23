#include "HUSKYLENS.h"
#include "Ambient.h"
#include <DFRobot_DF1201S.h>

#define PERIOD 10

WiFiClient client;
Ambient ambient;

const char* ssid     = "aterm-5459b0-g";
const char* password = "618fcf2fb4b02";

unsigned int channelId = 63925; // AmbientのチャネルID(数字)
const char* writeKey = "dd60de7e3debe1cd"; // ライトキー

char *data1;
char *data2;
char *data3;
bool ret;
int value = 0;

//SoftwareSerial DF1201SSerial(10, 11);  //RX  TX
HardwareSerial DF1201SSerial(2);  //RX=16  TX=17

HUSKYLENS huskylens;
//HUSKYLENS green line >> SDA; blue line >> SCL
void printResult(HUSKYLENSResult result);


char* ambient_name[] = {"", "なぎさちゃん", "かずゆきくん", "まさきくん"};
int play_nameid[] = {0, 1, 2, 3};
short int objectID = 0;

short int inID = 0;
int inRun = 0;

DFRobot_DF1201S DF1201S;

void setup(void) {
  Serial.begin(115200);
  ambientsetup();
  playsetup();
  Wire.begin();
  while (!huskylens.begin(Wire))
  {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
}
void ambientsetup() {
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ret = ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化
  Serial.println(ret);

}

void playsetup() {
  DF1201SSerial.begin(115200);
  while (!DF1201S.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }
  /*Set volume to 20*/
  DF1201S.setVol(/*VOL = */20);
  Serial.print("VOL:");
  /*Get volume*/
  Serial.println(DF1201S.getVol());
  /*Enter music mode*/
  DF1201S.switchFunction(DF1201S.MUSIC);
  /*Wait for the end of the prompt tone */
  delay(2000);
  /*Set playback mode to "repeat all"*/
  DF1201S.setPlayMode(DF1201S.SINGLE);
  Serial.print("PlayMode:");
  /*Get playback mode*/
  Serial.println(DF1201S.getPlayMode());
}

void ambientloop(char* id) {
  //data1 = "1";
  //data2 = "2";
  //data3 = "3";

  //ambient.set(1, data1);
  //ambient.set(2, data2);
  //  ret = ambient.set(1, *id);
  //ret = ambient.set(1, "かずゆきくん");
  ret = ambient.setcmnt(id);
  Serial.println(ret);

  ret = ambient.send(); // データをAmbientに送信
  Serial.println(ret);
  ret = Serial.println(ret);
  Serial.println(ret);
  Serial.println("***");

  delay(PERIOD * 1000);
}

void playloop(int id) {
  Serial.println("Start playing");
  /*Start playing*/
  DF1201S.playFileNum(id); // 1.wav の再生
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") +
                   result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  }
  else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") +
                   result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  }
  else {
    Serial.println("Object unknown!");
  }
}

bool objectcheck(HUSKYLENSResult result, short int *ID) {
  if (inID == result.ID) {
    inRun ++;
    if ((inRun >= 3) && (*ID != result.ID)) {
      *ID = result.ID;
      inID = 0;
      inRun = 0;
      return true;
    }
    else {
      return false;
    }
  }
  else {
    inID = result.ID;
    inRun = 0;
    return false;
  }
}
void stub() {
  Serial.println("stub!! ");
  HUSKYLENSResult result ;
  result.ID = 1;
  if (objectcheck(result, &objectID )) {
    ambientloop(ambient_name[objectID]);
    playloop(play_nameid[objectID]);
  }
  Serial.println(String() + F("result.ID: ") + result.ID + F(" objectID: ") + objectID + F(" inID: ") + inID + F(" inRun: ") + inRun );
  delay(1000);
}

void loop() {
  //  if (!huskylens.request()) stub();
  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if (!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
  else
  {
    Serial.println(F("###########"));
    while (huskylens.available())
    {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);

      if (objectcheck(result, &objectID )) {
        ambientloop(ambient_name[objectID]);
        playloop(play_nameid[objectID]);
      }
      Serial.println(String() + F("result.ID: ") + result.ID + F(" objectID: ") +
                     objectID + F(" inID: ") + inID + F(" inRun: ") + inRun );
    }
  }
}
