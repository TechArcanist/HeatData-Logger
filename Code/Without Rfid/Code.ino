#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP_Mail_Client.h>

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64   
#define OLED_RESET -1       
#define Button D7
#define Buzzer D4
#define led D6
#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 465
#define SENDER_EMAIL "printfatafat.corp@gmail.com"
#define SENDER_PASSWORD "xxxxxxxxxxxxxxxxx"
#define RECIPIENT_EMAIL "techpsit43@gmail.com"
#define RECIPIENT_NAME "THERMOMETER GUN"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const char* ssid = "PSITPSIT";
const char* password = "1234567890";

const char* host = "script.google.com";
const int httpsPort = 443;
const String GAS_ID = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
SMTPSession smtp;
WiFiClientSecure client;

void sendData(double obj);

void setup() {
  Serial.begin(115200);
  delay(500);
  mlx.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(Button, INPUT_PULLUP);
  pinMode(Buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  client.setInsecure();
}

void loop() {
  if (digitalRead(Button) == LOW) {
    digitalWrite(led, HIGH);
    double temp_amb = mlx.readAmbientTempC();
    double temp_obj = mlx.readObjectTempC();
    if(temp_obj>=37.50){
      digitalWrite(Buzzer, HIGH);
    }

    Serial.print("Room Temp = ");
    Serial.println(temp_amb);
    Serial.print("Object temp = ");
    Serial.println(temp_obj);

    display.clearDisplay();
    display.setCursor(25, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("");

    display.setCursor(10, 20);
    display.setTextSize(1);
    display.print("Body Temperature");

    display.setCursor(10, 40);
    display.setTextSize(1);
    display.print("     ");
    display.print(temp_obj);
    display.print((char)247);
    display.print("C");

    display.display();
    digitalWrite(Buzzer, LOW);
    digitalWrite(led, LOW);
    sendData(temp_obj);
    sendEmail(temp_amb, temp_obj);
  }
}

void sendData(double obj) {
  Serial.println("Sending data to Google Sheets");

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  String url = "/macros/s/" + GAS_ID + "/exec?object=" + String(obj);
  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  client.stop();
  Serial.println("Data sent successfully");
}



// String textMsg = "Hello Lavitra";

void sendEmail(double room, double obj) {

  ESP_Mail_Session session;
  session.server.host_name = SMTP_SERVER;
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "Thermometer Gun";
  message.sender.email = SENDER_EMAIL;
  message.subject = "Readings";
  message.addRecipient(RECIPIENT_NAME, RECIPIENT_EMAIL);

  String body = "Object Temperature = " + String(obj);
  message.text.content = body.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))
    return;

  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());

  else
    Serial.println("Email sent successfully!");

}
