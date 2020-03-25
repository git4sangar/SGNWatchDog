#!/usr/bin/python3.5
#sgn

from time import sleep
import json
import os
import socket

def send_packet(toip, port, pkt):
	sent	= 0
	sock_tx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	dest    = (toip, int(port))
	#print("Sending pkt to {0}".format(pkt))
	try:
		sent = sock_tx.sendto(pkt, dest)
	except socket.error as e:
		self.my_logger.info(os.strerror(e.errno))
	finally:
		sock_tx.close()
	return sent

if __name__ == "__main__":
	print("started main")
	heart_beat = {
		"command"			: "heart_beat",
		"process_name"		: "ocr",
		"pid_of_process"	: 444,
		"run_command"		: "/home/tstone10/sgn/bkup/private/projs/SGNBarc/technospurs/SmartMeter/ocr.py",
		"version"			: 1
	}
	while True:
		heart_beat["pid_of_process"] = os.getpid()
		print("sending packet {0}".format(json.dumps(heart_beat)))
		send_packet("127.0.0.1", 4951, json.dumps(heart_beat).encode())
		sleep(10);
