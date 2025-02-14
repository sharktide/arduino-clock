#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <dht_nonblocking.h>
#include <IRremote.h>
#include "pinmappings.h"  // Include pin mappings header

#define DHT_SENSOR_TYPE DHT_TYPE_11
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Initialize RTC and LCD with pins defined in pin_mappings.h
RTC_DS1307 rtc;
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

/*-----( Function )-----*/


void setup() {
  // Start Serial communication
  Serial.begin(9600);
  pinmode(lamp, OUTPUT)
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  // Initialize RTC
  if (!rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Couldn't find RTC");
    while (1);
  }

  // Check if RTC is running
  if (!rtc.isrunning()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC time to compile time
  }

  irrecv.enableIRIn();

  // Clear LCD after initialization
  lcd.clear();
  Serial.println("Send 'SET_TIME HH:MM:SS' to calibrate the time.");
  
  // Print the current time on LCD and Serial Monitor
  printTime();
}

void loop() {
  // Check if there's serial input for calibration
  if (Serial.available() > 0) {
    String input = Serial.readString();

    // If message received to set time, parse and adjust RTC
    if (input.startsWith("SET_TIME")) {
      // Extract the time from the message
      String timeStr = input.substring(9);  // Get substring after "SET_TIME"
      int hour = timeStr.substring(0, 2).toInt();  // Get the hour part
      int minute = timeStr.substring(3, 5).toInt();  // Get the minute part
      int second = timeStr.substring(6, 8).toInt();  // Get the second part

      // Set RTC time
      rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), hour, minute, second));
      Serial.println("Time set successfully!");

      // Print the updated time on both Serial and LCD
      printTime();
    }
  }

  if (irrecv.decode(&results)) // have we received an IR signal?

  {
    translateIR(); 
    irrecv.resume(); // receive the next value
  }  

  // Continuously display the time (no need for further serial messages)
  printTime();
  
  float temperature;
  float humidity;

  /* Measure temperature and humidity.  If the functions returns
    true, then a measurement is available. */
  if( measure_environment( &temperature, &humidity ) == true )
  {
    // Convert Celsius to Fahrenheit
    float temperatureF = (temperature * 9.0 / 5.0) + 32;

    Serial.print( "T = " );
    Serial.print( temperatureF, 1 );  // Print temperature in Fahrenheit
    Serial.print( " deg. F, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );

    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.setCursor(2, 1);
    lcd.print(temperatureF, 1 );
    lcd.setCursor(6, 1);
    lcd.print(" F");
    lcd.setCursor(9, 1);
    lcd.print("H:");
    lcd.setCursor(11, 1);
    lcd.print(humidity, 1 );
    lcd.print("%");
  }

  delay(920);  // Update every second
}

void printTime() {
  // Get current date and time from RTC
  DateTime now = rtc.now();

  // Display time on LCD
  lcd.setCursor(0, 0);
  if (now.hour() < 10) lcd.print("0");  // Leading zero for hours if needed
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");  // Leading zero for minutes if needed
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");  // Leading zero for seconds if needed
  lcd.print(now.second());

  // Display time on Serial Monitor
  Serial.print("Time: ");
  if (now.hour() < 10) Serial.print("0");  // Leading zero for hours if needed
  Serial.print(now.hour());
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");  // Leading zero for minutes if needed
  Serial.print(now.minute());
  Serial.print(":");
  if (now.second() < 10) Serial.print("0");  // Leading zero for seconds if needed
  Serial.println(now.second());
}

static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
  if( millis( ) - measurement_timestamp > 3000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }
  Serial.println("Not working :(");
  return( false );
}

void translateIR()  {

  switch(results.value)

  {
  case 0xFFA25D: Serial.println("POWER"); break;
  case 0xFFE21D: Serial.println("FUNC/STOP"); break;
  case 0xFF629D: Serial.println("VOL+"); break;
  case 0xFF22DD: Serial.println("FAST BACK");    break;
  case 0xFF02FD: Serial.println("PAUSE");    break;
  case 0xFFC23D: Serial.println("FAST FORWARD");   break;
  case 0xFFE01F: Serial.println("DOWN");    break;
  case 0xFFA857: Serial.println("VOL-");    break;
  case 0xFF906F: Serial.println("UP");    break;
  case 0xFF9867: Serial.println("EQ");    break;
  case 0xFFB04F: Serial.println("ST/REPT");    break;
  case 0xFF6897: Serial.println("0"); digitalWrite(lamp, HIGH);    break;
  case 0xFF30CF: Serial.println("1"); digitalWrite(lamp, LOW);    break;
  case 0xFF18E7: Serial.println("2");    break;
  case 0xFF7A85: Serial.println("3");    break;
  case 0xFF10EF: Serial.println("4");    break;
  case 0xFF38C7: Serial.println("5");    break;
  case 0xFF5AA5: Serial.println("6");    break;
  case 0xFF42BD: Serial.println("7");    break;
  case 0xFF4AB5: Serial.println("8");    break;
  case 0xFF52AD: Serial.println("9");    break;
  case 0xFFFFFFFF: Serial.println(" REPEAT");break;  

  default: 
    Serial.println(" other button   ");

  }// End Case



}
