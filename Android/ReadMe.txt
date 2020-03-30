sgn

About the project
-----------------
Using this app, one can set the WiFi Name (SSID) and password on a Raspberry Pi (RPi)
This app communicates with the "Watch Dog" app running on the RPi


Set Up
------
-> The android phone shall have the Hotspot name as "technospurs" and Hotspot password as "Welcome123$"
-> This makes sure, the RPi device gets connected to the android phone's hotspot when powered on


Sequence
--------
(01) Switch on the Mobile Hot Spot
(02) Switch on the Raspberr Pi and wait until it fully powers on
(03) Pressing the "PING" button sends the following json packet to RPi through UDP using Broadcast IP Address 
	{ "command" : "where_are_you" }
(04) The "Watch Dog" app responds back as follows with its WiFi IP address
	{ "command" : "where_are_you", "ip" : "192.168.43.78" }
(05) The same is set in the text-view on UI
(06) Now enter the Home-WiFi name and password
(07) Pressing "SET" button sends the following json packet to the IP address sent by RPi
	{ "command" : "wifi_details", "ssid" : "home-wifi-name", "psk" : "home-wifi-password" }
(08) RPi sets the WiFi credentials and sends the following response
	{ "command" : "wifi_details", "success" : true }
	Note: if false, the credentials are not set properly


Creating the project
--------------------
(01) Create a blank android project
(02) Add these java and xml files appropriately
