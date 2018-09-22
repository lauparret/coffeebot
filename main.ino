#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266WebServer.h>   // Include the WebServer library
MDNSResponder mdns;
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

// ---------------------CONSTANTS---------------------
const char* ssid     = "##################";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "##################";     // The password of the Wi-Fi network
#define toggle D1
#define smallcup D0
#define power D5
#define largecup D6
#define trig D7
#define echo D8
#define ledR D2
#define ledG D3
#define ledB D4
const int togglesmall = HIGH; 
bool makingcoffee = false;
bool isWaterhot = false;
unsigned long startuptimestamp;
unsigned long coffeetimestamp;
const unsigned long timedelta = 10000; // Time needed to heat the water
const unsigned long timedelta2 = 2000; // Time needed to poor coffee
const unsigned long relaisdelta = 1000; // How long the button will be pressed
const unsigned long ledblink = 500; // How long a led blink takes
long duration, cm;
const unsigned long mugthreshold = 15; // Threshold for mug distance in centimeters

// ---------------------SETUP---------------------
void setup() {
  // ---PINS--- Careful, pins D3 and D4 are high during boot, don't connect the relais to this, it won't boot
  pinMode(smallcup,OUTPUT);
  pinMode(power,OUTPUT);
  pinMode(largecup,OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(toggle,INPUT);
  digitalWrite(smallcup,LOW);
  digitalWrite(power,LOW);
  digitalWrite(largecup,LOW);
  digitalWrite(trig, LOW);
  digitalWrite(echo, LOW);
  
  // Start communication
  setLed('b'); // Set led to blue
  Serial.begin(9600); // Start serial communication with the computer
  WiFi.hostname("coffeemachine");
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting");
    delay(500);
  }
  Serial.println("Connected with IP:");
  Serial.println(WiFi.localIP());
  
  // ---MDNS---
  //if (mdns.begin("wifimodule")){
    //Serial.println("MDNS Started");
  //}
  
  // ---WEBSERVER---
  server.on("/", HTTP_GET, handleRoot);     // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/Coffee", HTTP_POST, handleCoffee);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.on("/Busy", HTTP_GET, handleBusy);
  server.on("/MugError", HTTP_GET, handleMugError);
  server.on("/Emergency", HTTP_POST, handleEmergency);
  server.on("/Stopped", HTTP_GET, handleStopped); 
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();
  Serial.println("HTTP server started");

  // Signal setup complete with green flash
  blinkLed('g');
  
}
  
  
// ---------------------LOOP---------------------
void loop() { 
  //digitalWrite(D1,HIGH);
  server.handleClient();

  if (makingcoffee == true) {
    checkcoffee();
  }
  }

//------ HTML -------------
const char MAIN_page[] = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>

<style type="text/css">
.form-style-1 {
    margin:8px auto;
    max-width: 1000px;
    padding: 20px 12px 10px 20px;
    font: 24px "Lucida Sans Unicode", "Lucida Grande", sans-serif;
}
.form-style-1 li {
    padding: 0;
    display: block;
    list-style: none;
    margin: 10px 0 0 0;
}
.form-style-1 label{
    margin:0 0 3px 0;
    padding:0px;
    display:block;
    font-weight: bold;
}
.form-style-1 input[type=text], 
.form-style-1 input[type=date],
.form-style-1 input[type=datetime],
.form-style-1 input[type=number],
.form-style-1 input[type=search],
.form-style-1 input[type=time],
.form-style-1 input[type=url],
.form-style-1 input[type=email],
textarea, 
select{
    box-sizing: border-box;
    -webkit-box-sizing: border-box;
    -moz-box-sizing: border-box;
    border:1px solid #BEBEBE;
    padding: 7px;
    margin:0px;
    -webkit-transition: all 0.30s ease-in-out;
    -moz-transition: all 0.30s ease-in-out;
    -ms-transition: all 0.30s ease-in-out;
    -o-transition: all 0.30s ease-in-out;
    outline: none;  
}
.form-style-1 input[type=text]:focus, 
.form-style-1 input[type=date]:focus,
.form-style-1 input[type=datetime]:focus,
.form-style-1 input[type=number]:focus,
.form-style-1 input[type=search]:focus,
.form-style-1 input[type=time]:focus,
.form-style-1 input[type=url]:focus,
.form-style-1 input[type=email]:focus,
.form-style-1 textarea:focus, 
.form-style-1 select:focus{
    -moz-box-shadow: 0 0 8px #88D5E9;
    -webkit-box-shadow: 0 0 8px #88D5E9;
    box-shadow: 0 0 8px #88D5E9;
    border: 1px solid #88D5E9;
}
.form-style-1 .field-divided{
    width: 49%;
}

.form-style-1 .field-long{
    width: 100%;
}
.form-style-1 .field-select{
    width: 100%;
}
.form-style-1 .field-textarea{
    height: 100px;
}
.form-style-1 input[type=submit], .form-style-1 input[type=button]{
    background: #4B99AD;
    padding: 8px 15px 8px 15px;
    border: none;
    color: #fff;
}
.form-style-1 input[type=submit]:hover, .form-style-1 input[type=button]:hover{
    background: #4691A4;
    box-shadow:none;
    -moz-box-shadow:none;
    -webkit-box-shadow:none;
}
.form-style-1 .required{
    color:red;
}
</style>

<h1 class="form-style-1">Welcome to the automatic coffee maker! </h1>

<form action="/Coffee" method="POST" class="form-style-1"><input type="submit" value="Make coffee" ></form>

</center>
</body>
</html>
)=====";

//-----------------------------------BUSY---------------------------------------
const char BUSY_page[] = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>

<style type="text/css">
.form-style-1 {
    margin:8px auto;
    max-width: 1000px;
    padding: 20px 12px 10px 20px;
    font: 24px "Lucida Sans Unicode", "Lucida Grande", sans-serif;
}
.form-style-1 li {
    padding: 0;
    display: block;
    list-style: none;
    margin: 10px 0 0 0;
}
.form-style-1 label{
    margin:0 0 3px 0;
    padding:0px;
    display:block;
    font-weight: bold;
}
.form-style-1 input[type=text], 
.form-style-1 input[type=date],
.form-style-1 input[type=datetime],
.form-style-1 input[type=number],
.form-style-1 input[type=search],
.form-style-1 input[type=time],
.form-style-1 input[type=url],
.form-style-1 input[type=email],
textarea, 
select{
    box-sizing: border-box;
    -webkit-box-sizing: border-box;
    -moz-box-sizing: border-box;
    border:1px solid #BEBEBE;
    padding: 7px;
    margin:0px;
    -webkit-transition: all 0.30s ease-in-out;
    -moz-transition: all 0.30s ease-in-out;
    -ms-transition: all 0.30s ease-in-out;
    -o-transition: all 0.30s ease-in-out;
    outline: none;  
}
.form-style-1 input[type=text]:focus, 
.form-style-1 input[type=date]:focus,
.form-style-1 input[type=datetime]:focus,
.form-style-1 input[type=number]:focus,
.form-style-1 input[type=search]:focus,
.form-style-1 input[type=time]:focus,
.form-style-1 input[type=url]:focus,
.form-style-1 input[type=email]:focus,
.form-style-1 textarea:focus, 
.form-style-1 select:focus{
    -moz-box-shadow: 0 0 8px #88D5E9;
    -webkit-box-shadow: 0 0 8px #88D5E9;
    box-shadow: 0 0 8px #88D5E9;
    border: 1px solid #88D5E9;
}
.form-style-1 .field-divided{
    width: 49%;
}

.form-style-1 .field-long{
    width: 100%;
}
.form-style-1 .field-select{
    width: 100%;
}
.form-style-1 .field-textarea{
    height: 100px;
}
.form-style-1 input[type=submit], .form-style-1 input[type=button]{
    background: #ff0000;
    padding: 8px 15px 8px 15px;
    border: none;
    color: #fff;
}
.form-style-1 input[type=submit]:hover, .form-style-1 input[type=button]:hover{
    background: #000000;
    box-shadow:none;
    -moz-box-shadow:none;
    -webkit-box-shadow:none;
}
.form-style-1 .required{
    color:red;
}
</style>

<h1 class="form-style-1">The coffeebot is preparing your beverage! </h1>

<form action="/Emergency" method="POST" class="form-style-1"><input type="submit" value="Emergency stop" ></form>

</center>
</body>
</html>
)=====";

//------------------------------------STOPPED ----------------------------------------------------------------
const char STOPPED_page[] = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>

<style type="text/css">
.form-style-1 {
    margin:8px auto;
    max-width: 1000px;
    padding: 20px 12px 10px 20px;
    font: 24px "Lucida Sans Unicode", "Lucida Grande", sans-serif;
}
.form-style-1 li {
    padding: 0;
    display: block;
    list-style: none;
    margin: 10px 0 0 0;
}
.form-style-1 label{
    margin:0 0 3px 0;
    padding:0px;
    display:block;
    font-weight: bold;
}
.form-style-1 input[type=text], 
.form-style-1 input[type=date],
.form-style-1 input[type=datetime],
.form-style-1 input[type=number],
.form-style-1 input[type=search],
.form-style-1 input[type=time],
.form-style-1 input[type=url],
.form-style-1 input[type=email],
textarea, 
select{
    box-sizing: border-box;
    -webkit-box-sizing: border-box;
    -moz-box-sizing: border-box;
    border:1px solid #BEBEBE;
    padding: 7px;
    margin:0px;
    -webkit-transition: all 0.30s ease-in-out;
    -moz-transition: all 0.30s ease-in-out;
    -ms-transition: all 0.30s ease-in-out;
    -o-transition: all 0.30s ease-in-out;
    outline: none;  
}
.form-style-1 input[type=text]:focus, 
.form-style-1 input[type=date]:focus,
.form-style-1 input[type=datetime]:focus,
.form-style-1 input[type=number]:focus,
.form-style-1 input[type=search]:focus,
.form-style-1 input[type=time]:focus,
.form-style-1 input[type=url]:focus,
.form-style-1 input[type=email]:focus,
.form-style-1 textarea:focus, 
.form-style-1 select:focus{
    -moz-box-shadow: 0 0 8px #88D5E9;
    -webkit-box-shadow: 0 0 8px #88D5E9;
    box-shadow: 0 0 8px #88D5E9;
    border: 1px solid #88D5E9;
}
.form-style-1 .field-divided{
    width: 49%;
}

.form-style-1 .field-long{
    width: 100%;
}
.form-style-1 .field-select{
    width: 100%;
}
.form-style-1 .field-textarea{
    height: 100px;
}
.form-style-1 input[type=submit], .form-style-1 input[type=button]{
    background: #4B99AD;
    padding: 8px 15px 8px 15px;
    border: none;
    color: #fff;
}
.form-style-1 input[type=submit]:hover, .form-style-1 input[type=button]:hover{
    background: #4691A4;
    box-shadow:none;
    -moz-box-shadow:none;
    -webkit-box-shadow:none;
}
.form-style-1 .required{
    color:red;
}
</style>

<h1 class="form-style-1">Emergency stop activated!</h1>

<form action="/" method="GET" class="form-style-1"><input type="submit" value="Return to main page" ></form>

</center>
</body>
</html>
)=====";

//------------------------------------MUGERROR ----------------------------------------------------------------
const char MUGERROR_page[] = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>

<style type="text/css">
.form-style-1 {
    margin:8px auto;
    max-width: 1000px;
    padding: 20px 12px 10px 20px;
    font: 24px "Lucida Sans Unicode", "Lucida Grande", sans-serif;
}
.form-style-1 li {
    padding: 0;
    display: block;
    list-style: none;
    margin: 10px 0 0 0;
}
.form-style-1 label{
    margin:0 0 3px 0;
    padding:0px;
    display:block;
    font-weight: bold;
}
.form-style-1 input[type=text], 
.form-style-1 input[type=date],
.form-style-1 input[type=datetime],
.form-style-1 input[type=number],
.form-style-1 input[type=search],
.form-style-1 input[type=time],
.form-style-1 input[type=url],
.form-style-1 input[type=email],
textarea, 
select{
    box-sizing: border-box;
    -webkit-box-sizing: border-box;
    -moz-box-sizing: border-box;
    border:1px solid #BEBEBE;
    padding: 7px;
    margin:0px;
    -webkit-transition: all 0.30s ease-in-out;
    -moz-transition: all 0.30s ease-in-out;
    -ms-transition: all 0.30s ease-in-out;
    -o-transition: all 0.30s ease-in-out;
    outline: none;  
}
.form-style-1 input[type=text]:focus, 
.form-style-1 input[type=date]:focus,
.form-style-1 input[type=datetime]:focus,
.form-style-1 input[type=number]:focus,
.form-style-1 input[type=search]:focus,
.form-style-1 input[type=time]:focus,
.form-style-1 input[type=url]:focus,
.form-style-1 input[type=email]:focus,
.form-style-1 textarea:focus, 
.form-style-1 select:focus{
    -moz-box-shadow: 0 0 8px #88D5E9;
    -webkit-box-shadow: 0 0 8px #88D5E9;
    box-shadow: 0 0 8px #88D5E9;
    border: 1px solid #88D5E9;
}
.form-style-1 .field-divided{
    width: 49%;
}

.form-style-1 .field-long{
    width: 100%;
}
.form-style-1 .field-select{
    width: 100%;
}
.form-style-1 .field-textarea{
    height: 100px;
}
.form-style-1 input[type=submit], .form-style-1 input[type=button]{
    background: #4B99AD;
    padding: 8px 15px 8px 15px;
    border: none;
    color: #fff;
}
.form-style-1 input[type=submit]:hover, .form-style-1 input[type=button]:hover{
    background: #4691A4;
    box-shadow:none;
    -moz-box-shadow:none;
    -webkit-box-shadow:none;
}
.form-style-1 .required{
    color:red;
}
</style>

<h1 class="form-style-1">No mug detected! </h1>

<form action="/" method="GET" class="form-style-1"><input type="submit" value="Return to main page" ></form>

</center>
</body>
</html>
)=====";
//String htmlCode = "<form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Toggle LED\"></form>";


// ---------------------------------------- PAGE FUNCTIONS -----------------------


void handleRoot() {                         // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", MAIN_page);
}

void handleCoffee() {                          // If a POST request is made to URI /LED
  Serial.println("Toggle detected");
  
  if (makingcoffee == false){
    if (checkmug()==true) {
      blinkLed('g');                            // Blink green on startup
      server.sendHeader("Location","/Busy");        // Add a header to respond with a new location for the browser to go to the home page again
      server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
      makingcoffee = true;
      Serial.println("Making Coffee!"); 
      startup();
      
    }
    else {
      server.sendHeader("Location","/MugError");        // Add a header to respond with a new location for the browser to go to the home page again
      server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
      Serial.println("Coffee failed");
      blinkLed('r');                            // Blink red on failed attempt
    }  
  }
  else{
    server.sendHeader("Location","/Busy");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303); 
    //server.send(404, "text/plain", "404: Already making coffee!");
    Serial.println("Already making coffee!");
    blinkLed('c');            //Signal received, but busy -> blink cyan 
  }
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void handleBusy(){
  server.send(200, "text/html", BUSY_page);
  //server.send(404, "text/plain", "Already making coffee!"); 
}
void handleMugError(){
  server.send(200, "text/html", MUGERROR_page);
  //server.send(404, "text/plain", "No mug detected!"); 
}

void handleEmergency(){
    if (makingcoffee == true) {
      server.sendHeader("Location","/Stopped");       
      server.send(303);
      //server.send(404, "text/plain", "Emergency stop activated!");
      poweroff();
      makingcoffee = false;
      isWaterhot = false;
      blinkLed('r');
      blinkLed('r');
  }
  else {
    server.sendHeader("Location","/");       
    server.send(303);
  }
}

void handleStopped(){
  server.send(200, "text/html", STOPPED_page);
}

// ----------------------------- Coffee funtions ------------------------------------

bool checkmug() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(echo, HIGH);
  cm = (duration/2) *0.0343; // deltax[cm] = (deltatime[microseconds] * 10^-6) * (343*10^2 cm/s) 
  Serial.print(duration);
  Serial.print("duration");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

  if (cm < mugthreshold){
    return true;
  }
  else {
    return false;
  }
  }

bool checktoggle(){
  pinMode(toggle,INPUT_PULLUP);
  delay(5);
  int state = digitalRead(toggle);
  delay(5);
  pinMode(toggle,INPUT);
  return state;
}

void startup() {
  digitalWrite(power,HIGH);
  delay(relaisdelta);
  digitalWrite(power,LOW);
  startuptimestamp = millis();
  Serial.println("Started!");
}
void poweroff(){
  digitalWrite(power,HIGH);
  delay(relaisdelta);
  digitalWrite(power,LOW);
  Serial.println("Power off");
}

//void emergencystop(){ // Only to be used when the machine is already on and busy!
//  if (makingcoffee == true) {
//    //server.send(404, "text/plain", "Emergency stop activated!");
//    poweroff();
//    makingcoffee = false;
//    isWaterhot = false;
//    blinkLed('r');
//    blinkLed('r');
//  }
//  else {
//    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
//    server.send(303);
//  }
//}

void makecoffee() {
  if (checktoggle() == togglesmall){
    digitalWrite(smallcup,HIGH);
    delay(relaisdelta);
    digitalWrite(smallcup,LOW);
    coffeetimestamp = millis();
    Serial.println("Small cup started");
  }
  else {
    digitalWrite(largecup,HIGH);
    delay(relaisdelta);
    digitalWrite(largecup,LOW);
    coffeetimestamp = millis();
    Serial.println("Large cup started");
  }
   
}

void checkcoffee(){
  setLed('y');
  if ( (isWaterhot == false) && (millis() > (startuptimestamp + timedelta))) {
    Serial.println("Water is heated");
    isWaterhot = true;
    makecoffee();
    
  }
  else if ((isWaterhot == true) && (millis() > (coffeetimestamp + timedelta2))){
    Serial.println("Done");
    makingcoffee = false;
    isWaterhot = false;
    poweroff();
    blinkLed('g');
    server.sendHeader("Location","/"); 
    server.send(401);
  }
}


// --------------------------LED ------------------------------
void blinkLed(char color) {
  setLed(color);
  delay(ledblink);
  setLed('x');
}

void setLed(char color) {
  switch (color){
    case 'r': //red
      setColor(1,0,0);
      break;
    case 'g': // green
      setColor(0,1,0);
      break;
    case 'b': // blue
      setColor(0,0,1);
      break;
    case 'w': //white
      setColor(1,1,1);
      break;
    case 'c': //cyan
      setColor(0,1,1);
      break;
    case 'y': //yellow
      setColor(1,1,0);
      break;
    case 'p': //purple
      setColor(1,0,1);
      break;
    case 'f': // Turn off
      setColor(0,0,0);
      break;
    default: // Turn off as default
      setColor(0,0,0);
      break;
  }
}

void setColor(int red, int green, int blue) {
  red = 1- red;
  green = 1- green;
  blue = 1- blue;

  digitalWrite(ledR,red);
  digitalWrite(ledG,green);
  digitalWrite(ledB,blue);
}
