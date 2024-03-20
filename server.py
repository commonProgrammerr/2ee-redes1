import socket
import time
from ctypes import c_uint16 as uint16
from ctypes import c_uint8 as uint8
# from dateutil import tz
# from datetime import datetime


UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

UDPServerSocket.bind(("0.0.0.0", 3030))
print("UDP server up and listening")

while(True):

    (message , address) = UDPServerSocket.recvfrom(1024)
    print(f'New client: {address}')
    time_offset = int.from_bytes(message, 'big', signed=True)
    tm = time.gmtime(time.time() + (3600 *time_offset ))
    # tm = time.localtime()
    data = bytearray()
    data.append(tm.tm_mday)
    data.append(tm.tm_mon)
    data.append(tm.tm_hour)
    data.append(tm.tm_min)
    data.append(tm.tm_sec)
    data.append(tm.tm_wday)
    data.append(uint8(time_offset).value)
    data.append(0)
    for b in uint16(tm.tm_year).value.to_bytes(2, 'little'):
      data.append(b)
    UDPServerSocket.sendto(data, address)