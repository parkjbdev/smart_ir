#include "buzzer.h"
//"/Users/parkjb/Library/Arduino15/packages/esp32/tools/esp32-arduino-libs/idf-release_v5.1-632e0c2a/esp32/**/*include/**",

#include "matrix.h"

#define SEND_PWM_BY_TIMER
#define IR_RECEIVE_PIN 16
#define IR_SEND_PIN 17
#include <IRremote.hpp>

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_USERNAME "ajouesp"
#define MQTT_PASSWORD "password"

#define MQTT_REGISTER_TOPIC "topic/register_hex"
#define MQTT_CONTROL_TOPIC "topic/control"
#ifdef FEATURE_MATRIX
#define MQTT_MATRIX_TOPIC "topic/matrix"
#define MQTT_FACE_TOPIC "topic/face"
#endif
#ifdef FEATURE_BUZZER
#define MQTT_BUZZER_TOPIC "topic/buzzer"
#endif

char clientId[50];

// WiFi Connection
char SSID_LIST[][20] = {"iPhone", "PARKJB"};
char PASSWORD_LIST[][20] = {"aldkiozs92", "sT?x1s=lp_op"};

void init_wifi() {
  int n = WiFi.scanNetworks();

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < sizeof(SSID_LIST) / sizeof(SSID_LIST[0]); j++) {
      if (WiFi.SSID(i) == SSID_LIST[j]) {
        WiFi.begin(SSID_LIST[j], PASSWORD_LIST[j]);
        Serial.print("[WiFi] Connecting to ");
        Serial.print(SSID_LIST[j]);
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          delay(1000);
        }
        Serial.println();
        Serial.println("[WiFi] Connected to the WiFi network");
        Serial.print("[WiFi] IP Address: ");
        Serial.println(WiFi.localIP());
        return;
      }
    }
  }
}

// MQTT Connection
void connect_mqtt() {
  Serial.print("Attempting MQTT connection...");
  long r = random(1000);
  sprintf(clientId, "clientId-%ld", r);
  if (client.connect(clientId, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(clientId);
    Serial.println(" connected");
    client.subscribe(MQTT_FACE_TOPIC);
    client.subscribe(MQTT_CONTROL_TOPIC);
#ifdef FEATURE_MATRIX
    client.subscribe(MQTT_MATRIX_TOPIC);
#endif
#ifdef FEATURE_BUZZER
    client.subscribe(MQTT_BUZZER_TOPIC);
#endif
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    delay(5000);
  }
}

void mqttCallback(char *topic, byte *message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String stMessage;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    stMessage += (char)message[i];
  }
  Serial.println();
#ifdef FEATURE_MATRIX
  if (String(topic) == MQTT_MATRIX_TOPIC) {
    marquee_text(stMessage.c_str());
  }
#endif
#ifdef FEATURE_BUZZER
  if (String(topic) == MQTT_BUZZER_TOPIC) {
    play_canon();
  }
#endif
  if (String(topic) == MQTT_FACE_TOPIC) {
    draw_face(stMessage.toInt());
    Serial.println("Test message received");
  }

  if (String(topic) == MQTT_CONTROL_TOPIC) {
    // send ir
    JsonDocument ir_json;
    deserializeJson(ir_json, stMessage);

    String tmp_protocol = ir_json["protocol"];
    String tmp_address = ir_json["address"];
    String tmp_command = ir_json["command"];
    String tmp_extra = ir_json["extra"];
    String tmp_numberOfBits = ir_json["numberOfBits"];
    String tmp_flags = ir_json["flags"];
    String tmp_decodedRawData = ir_json["decodedRawData"];

    int msg_len = tmp_decodedRawData.length() + 1;
    char buf[msg_len];
    tmp_decodedRawData.toCharArray(buf, msg_len);

    decode_type_t protocol = decode_type_t(tmp_protocol.toInt());
    uint16_t address = tmp_address.toInt();
    uint16_t command = tmp_command.toInt();
    uint16_t extra = tmp_extra.toInt();
    uint16_t numberOfBits = tmp_numberOfBits.toInt();
    uint8_t flags = tmp_flags.toInt();
    IRRawDataType decodedRawData = strtoull(buf, NULL, 10);

    IRData ir_data = {
        .protocol = protocol,
        .address = address,
        .command = command,
        .extra = extra,
        .decodedRawData = decodedRawData,
        .numberOfBits = numberOfBits,
        .flags = flags,
    };

    IrSender.write(&ir_data);
    printIRResultShort(&Serial, &ir_data, false);
  }
}

// IR Remote
void handle_ir_rcv() {
  IrReceiver.printIRResultShort(&Serial);
  if (IrReceiver.decodedIRData.rawDataPtr->rawlen < 4) {
    Serial.print("Ignore data with rawlen=");
    Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawlen);
  } else if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
    Serial.println("Ignore repeat");
  } else if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
    Serial.println("Ignore autorepeat");
  } else if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED) {
    Serial.println("Ignore parity error");
  } else if (IrReceiver.decodedIRData.protocol == decode_type_t(0)) {
    Serial.println("Ignore unknown protocol");
  } else {
    // Serialize IRData and publish to MQTT
    JsonDocument payload;

    payload["protocol"] = IrReceiver.decodedIRData.protocol;
    payload["address"] = IrReceiver.decodedIRData.address;
    payload["command"] = IrReceiver.decodedIRData.command;
    payload["extra"] = IrReceiver.decodedIRData.extra;
    payload["numberOfBits"] = IrReceiver.decodedIRData.numberOfBits;
    payload["flags"] = IrReceiver.decodedIRData.flags;
    payload["decodedRawData"] = IrReceiver.decodedIRData.decodedRawData;

    String message = "";
    serializeJson(payload, message);

    int msg_len = message.length() + 1;
    char buf[msg_len];
    message.toCharArray(buf, msg_len);
    client.publish(MQTT_REGISTER_TOPIC, buf);
  }
  IrReceiver.resume();
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(4000);

#ifdef FEATURE_MATRIX
  init_matrix();
#endif
#ifdef FEATURE_BUZZER
  init_buzzer();
#endif

  // Connect to WiFi
  init_wifi();

  // Connect to MQTT
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);

  // Initialize IR Receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.print("Ready to receive IR signals of protocols: ");
  printActiveIRProtocols(&Serial);
  Serial.print("at pin ");
  Serial.println(IR_RECEIVE_PIN);

  // Initialize IR Sender
  IrSender.begin();
  Serial.print("Send IR signals at pin ");
  Serial.println(IR_SEND_PIN);
}

void loop() {
  while (!client.connected())
    connect_mqtt();

  if (IrReceiver.decode())
    handle_ir_rcv();

  client.loop();
}
