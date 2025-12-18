import argparse
import re 
import struct

parser = argparse.ArgumentParser(description='Convert C network to binary')
parser.add_argument('-f', nargs='?',type = str, default="vela_H128.bin", help="Binary file")

parser.add_argument('c', nargs=1,help="C network file")

args = parser.parse_args()

BEFORE=1
INNETWORK=2

state = 1 

network = bytes()

with open(args.c[0],"r") as f:
    for line in f:
        if state == BEFORE:
            if re.match(r'^static const uint8_t nn_model.*$',line):
                state = INNETWORK
        elif state == INNETWORK:
            bytes = [int(x,16) for x in re.findall(r'0x[0-9a-fA-F]{2}',line)]
            if bytes:
               data = struct.pack(f'<{len(bytes)}B', *bytes)
               network += data
            if re.search(r'};',line):
                 break
            
with open(args.f,"wb") as f:
    f.write(network)

