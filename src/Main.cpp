#include <Arduino.h>
#include <MCP7941x.h>
#include <Wire.h>
#include <LowPower.h>

#define LINE_BUF_SIZE 128   //Maximum input string length
#define ARG_BUF_SIZE 64     //Maximum argument string length
#define MAX_NUM_ARGS 8      //Maximum number of arguments
 
bool error_flag = false;
 
char line[LINE_BUF_SIZE];
char args[MAX_NUM_ARGS][ARG_BUF_SIZE];

uint8_t address = 0x6F;

//Function declarations
int cmd_read();
int cmd_write();
int cmd_exit();
 
//List of functions pointers corresponding to each command
int (*commands_func[])(){
    &cmd_read,
    &cmd_write,
    &cmd_exit
};
 
//List of command names
const char *commands_str[] = {
    "r",
    "w",
    "exit"
};
  
int num_commands = sizeof(commands_str) / sizeof(char *);

uint8_t readData(uint8_t reg) 
{
	Wire.beginTransmission(address);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(address, uint8_t(1));
	uint8_t value = Wire.read();
	Wire.endTransmission();
	return value;
}


void writeData(uint8_t reg, uint8_t value) 
{
	Wire.beginTransmission(address);
	Wire.write(reg);
	Wire.write(value);
	Wire.endTransmission();
}

void read_line(){
    String line_string;
 
    while(!Serial.available());
 
    if(Serial.available()){
        line_string = Serial.readStringUntil('\n');
        if(line_string.length() < LINE_BUF_SIZE){
          line_string.toCharArray(line, LINE_BUF_SIZE);
          Serial.println(line_string);
        }
        else{
          Serial.println("Input string too long.");
          error_flag = true;
        }
    }
}

void parse_line(){
    char *argument;
    int counter = 0;
 
    argument = strtok(line, " ");
 
    while((argument != NULL)){
        if(counter < MAX_NUM_ARGS){
            if(strlen(argument) < ARG_BUF_SIZE){
                strcpy(args[counter],argument);
                argument = strtok(NULL, " ");
                counter++;
            }
            else{
                Serial.println("Input string too long.");
                error_flag = true;
                break;
            }
        }
        else{
            break;
        }
    }
}

int execute(){  
    for(int i=0; i<num_commands; i++){
        if(strcmp(args[0], commands_str[i]) == 0){
            return(*commands_func[i])();
        }
    }
 
    Serial.println("Invalid command. Type \"help\" for more.");
    return 0;
}
void my_cli(){
    Serial.print("> ");
 
    read_line();
    if(!error_flag){
        parse_line();
    }
    if(!error_flag){
        execute();
    }
 
    memset(line, 0, LINE_BUF_SIZE);
    memset(args, 0, sizeof(args[0][0]) * MAX_NUM_ARGS * ARG_BUF_SIZE);
 
    error_flag = false;
}

MCP7941x rtc;
void setup(void) 
{
    Serial.begin(9600);
    Wire.begin();
    pinMode(3, INPUT_PULLUP);
    digitalWrite(9, HIGH);
    Serial.println("Starting");
    MCP7941x::dateTime dt;
    dt.year = 2022;
    dt.month = 1;
    dt.day = 9;
    dt.hours = 19;
    dt.fmt12H = true;
    dt.AM = true;
    dt.minutes = 59;
    dt.seconds = 52;
    rtc.setClock(&dt);
    rtc.startClock();
}

void wakeUp(void) {

}

void loop(void) 
{
    MCP7941x::dateTime dt;
    rtc.getClock(&dt);
    char buffer[20];
    if(dt.fmt12H)
        sprintf(buffer, "%02d:%02d:%02d(%s) %02d/%02d/%04d", dt.hours, dt.minutes, dt.seconds, dt.AM?"AM":"PM",  dt.day, dt.month, dt.year);
    else
        sprintf(buffer, "%02d:%02d:%02d %02d/%02d/%04d", dt.hours, dt.minutes, dt.seconds, dt.day, dt.month, dt.year);
    Serial.println(buffer);
    delay(1000);
    /*
    attachInterrupt(digitalPinToInterrupt(3), wakeUp, FALLING);
    LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
    detachInterrupt(0);
    my_cli();
    */
}



int cmd_read() {
    uint8_t reg = (uint8_t)strtol(args[1], 0, 16);
    Serial.print("Will read ");Serial.print(reg);Serial.print("...");
    uint8_t val = readData(reg);
    Serial.print(val, HEX);
    Serial.print(" = ");
    Serial.println(val, BIN);
    return 0;
}

int cmd_write() {
    Serial.println("Will write ");
    uint8_t val = (uint8_t)strtol(args[1], 0, 16);
    uint8_t reg = (uint8_t)strtol(args[2], 0, 16);
    Serial.print(val);Serial.print(" to ");Serial.print(reg);
    writeData(reg, val);
    Serial.println("... done");
    return 0;
}

int cmd_exit() {
    Serial.println("Will exit");
    return 0;
}

