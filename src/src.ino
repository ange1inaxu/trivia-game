#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>


TFT_eSPI tft = TFT_eSPI();

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 3500; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
int timer;

char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
char response_buffer_copy[OUT_BUFFER_SIZE];
const char USER[] = "angelx";


uint8_t BUTTON1 = 45;
uint8_t BUTTON2 = 39;
uint8_t BUTTON3 = 38;

uint8_t state;
const uint8_t IDLE = 1;
const uint8_t QUESTION_DISPLAY = 2;
const uint8_t USER_ANSWER = 3;
const uint8_t UPDATE = 4;
const uint8_t END_GAME = 5;

char output[1000] = "";
char question[1000] = "";
char answer[10] = "";
int correct = 0;
int incorrect = 0;

char network[] = "MIT";
char password[] = "";

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for Serial to show up

  connect_wifi();
  
  tft.init(); //initialize the screen
  tft.setRotation(2); //set rotation for our layout
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  //four pins needed: two inputs, two outputs. Set them up appropriately:
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  state = IDLE;
  sprintf(output, "Long press (button 1) to start the game!");
}


void get_question(char* response, char* query) {
  char request_buffer[200];
  sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/angelx15/trivia.py?scoreboard=%s HTTP/1.1\r\n", query);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "\r\n"); //new line from header to body
  char host[] = "608dev-2.net";
  do_http_GET(host, request_buffer, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

//enum for button states
enum button_state {S0, S1, S2, S3, S4};

class Button{
  public:
  uint32_t S2_start_time;
  uint32_t button_change_time;    
  uint32_t debounce_duration;
  uint32_t long_press_duration;
  uint8_t pin;
  uint8_t flag;
  uint8_t button_pressed;
  button_state state; // This is public for the sake of convenience
  Button(int p) {
  flag = 0;  
    state = S0;
    pin = p;
    S2_start_time = millis(); //init
    button_change_time = millis(); //init
    debounce_duration = 10;
    long_press_duration = 1000;
    button_pressed = 0;
  }
  void read() {
    uint8_t button_val = digitalRead(pin);  
    button_pressed = !button_val; //invert button
  }
  int update() {
    read();
    flag = 0;
    if (state==S0) {
      if (button_pressed) {
        state = S1;
        button_change_time = millis();
      }
    } else if (state==S1) {
      if (button_pressed && millis()-button_change_time >= debounce_duration){
        state = S2;
        S2_start_time = millis();
      } else if (!button_pressed){
        state = S0;
        button_change_time = millis();
      }
    } else if (state==S2) {
      if (button_pressed && millis()-S2_start_time >= long_press_duration){
        state = S3;
      } else if (!button_pressed){
        state = S4;
        button_change_time = millis();
      }
    } else if (state==S3) {
      if (!button_pressed){
        state = S4;
        button_change_time = millis();
      }
    } else if (state==S4) {        
      if (!button_pressed && millis()-button_change_time >= debounce_duration){
        state = S0;
        if (millis()-S2_start_time < long_press_duration){
          flag = 1;
        } else if (millis()-S2_start_time >= long_press_duration){
          flag = 2;
        }
      } else if (button_pressed && millis()-S2_start_time < long_press_duration){
        state = S2;
        button_change_time = millis();
      } else if (button_pressed && millis()-S2_start_time >= long_press_duration){
        state = S3;
        button_change_time = millis();
      }
    }
    return flag;
  }
};

Button button1(BUTTON1); //button object!
Button button2(BUTTON2); //button object!
Button button3(BUTTON3); //button object!


//main body of code
void loop() {
  //get button values
  int bv1 = button1.update();
  int bv2 = button2.update();
  int bv3 = button3.update();

  if (state == IDLE){
    tft.setCursor(0, 0, 1);
    tft.println(output);

    //Long press button 1
    if (bv1 == 2){
      state = QUESTION_DISPLAY;
    }
    
  } else if (state == QUESTION_DISPLAY){
    get_question(request, "False");
    Serial.println(request);

    //parse for question and answer
    char delim[2] = "\n";
    char* token;
    token = strtok(request, delim);
    strcpy(question, token);
    token = strtok(NULL, delim);
    strcpy(answer, token);

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 1);
    memset(output, 0, sizeof(output));
    sprintf(output, "Correct: %d \nIncorrect: %d \nScore: %d \n\nQuestion: %s \n\nPress Button 2 (False) or Button 3 (True)                                                                               ", correct, incorrect, correct-incorrect, question);
    tft.println(output);

    state = USER_ANSWER;
    
  } else if (state == USER_ANSWER){

    memset(output, 0, sizeof(output));
    // short press b2 (False)
    if (bv2 == 1){
      state = UPDATE;
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      
      if (strcmp(answer, "False")==0){
        correct += 1;
        sprintf(output, "Correct: %d \nIncorrect: %d \nScore: %d \n\nCORRECT! \n\nLong press Button 1 to end game or short press Button 1 to continue                                                    ", correct, incorrect, correct-incorrect);
        tft.println(output);
      } else {
        incorrect += 1;
        sprintf(output, "Correct: %d \nIncorrect: %d \nScore: %d \n\nINCORRECT! \n\nLong press Button 1 to end game or short press Button 1 to continue                                                  ", correct, incorrect, correct-incorrect);
        tft.println(output);
      }
    }

    // short press b3 (True)
    else if (bv3 == 1){
      state = UPDATE;
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      
      if (strcmp(answer, "True")==0){
        correct += 1;
        sprintf(output, "Correct: %d \nIncorrect: %d \nScore: %d \n\nCORRECT! \n\nLong press Button 1 to end game or short press Button 1 to continue                                                    ", correct, incorrect, correct-incorrect);
        tft.println(output);
      } else {
        incorrect += 1;
        sprintf(output, "Correct: %d \nIncorrect: %d \nScore: %d \n\nINCORRECT! \n\nLong press Button 1 to end game or short press Button 1 to continue                                                  ", correct, incorrect, correct-incorrect);
        tft.println(output);
      }
    }
    
  } else if (state == UPDATE) {
    if (bv1 == 1){
      state = QUESTION_DISPLAY;
    } else if (bv1 == 2){
      state = END_GAME;
    }
    
  } else if (state == END_GAME){
    state = IDLE;
    memset(output, 0, sizeof(output));
    tft.fillScreen(TFT_BLACK);
    
    char body[100]; //for body
    sprintf(body,"scoreboard=\"True\"&user=%s&score=%d",USER,correct-incorrect);//generate body, posting to User, 1 step
    int body_len = strlen(body); //calculate body length (for header reporting)
    sprintf(request_buffer,"POST https://608dev-2.net/sandbox/sc/angelx15/trivia.py HTTP/1.1\r\n");
    strcat(request_buffer,"Host: 608dev-2.net\r\n");
    strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
    sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
    strcat(request_buffer,"\r\n"); //new line from header to body
    strcat(request_buffer,body); //body
    strcat(request_buffer,"\r\n"); //new line
    Serial.println(request_buffer);
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);

    Serial.println("\nRESPONSE BUFFER:\n");
    Serial.println(response_buffer); //viewable in Serial Terminal

    sprintf(output, "SCOREBOARD: \n%s \n\nLong press (button 1) to start the game!", response_buffer);

    //reset local variables
    correct = 0;
    incorrect = 0;
  }
}
