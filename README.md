##Arduino/ESP8266 library for Mitsubishi Aircon/HVAC/Heatpump via serial connector CN105
#####Working but under development!



The library provides an encode/decode class for protocol actions and also a controller class for simplified interaction from the user code.

It was designed for use with (most/some?) recent mitsubishi heat pump/air con/hvac units and possibly others. The unit requires the CN105 connector which exposes a UART, 5v and 12v.

I used an Adafruit Huzzah Feather ESP8266, which is a 3v chip but the feather has an onboard regulator and the RX has a 5v level shifter. TX line from the feather only supplies 3v but the air con is happy with that.

Might work with other Arduino boards but you may need voltage regulators or adjust some of the library.

Made possible by a good write up by Hadley Rich (https://nicegear.co.nz/blog/hacking-a-mitsubishi-heat-pump-air-conditioner/)

It's designed to be as simple to use as possible, there are only 4 methods:

    MitsuAc ac(&Serial);
    ac.connect(); // send an init data packet  
    ac.monitor(); // service the control loop of the library 
    int putSettingsJson(const char* jsonSettings, size_t len); //send one or more parameters to the unit 
    void getSettingsJson(char* settings, size_t len); //get the last known settings from the unit
    
    
One might receive a string such as:

    {'power':'on','mode':'auto','fan':'1','vane':'3','widevane':'center','temp':22','roomTemp':22}

And control the unit with:

    {'power':'on;}
or

    {'power':'on','mode':'auto','fan':'1','vane':'3'}

    


Credits:
Raspberry Pi script and protocol originally reverse engineered by Hadley Rich (@hadleyrich) (http://nice.net.nz)
