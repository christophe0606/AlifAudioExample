import argparse
import re 
import struct
import hashlib

parser = argparse.ArgumentParser(description='Convert C network to binary')
parser.add_argument('-f', nargs='?',type = str, default="networks.bin", help="Binary file")
parser.add_argument('-x', nargs='?',type = int, default=0xC0000000, help="Ext mem start address")

parser.add_argument('c', nargs=1,help="C network file")

args = parser.parse_args()

ALIGN=16
BEFORE=1
INNETWORK=2


def get_network(filename):
    if re.search(r'\.bin$',filename):
        with open(filename,"rb") as f:
            return f.read()
    state = 1 
    network = bytes()
    
    with open(filename,"r") as f:
        for line in f:
            if state == BEFORE:
                if re.match(r'^static const uint8_t nn_model.*$',line):
                    state = INNETWORK
            elif state == INNETWORK:
                line_bytes = [int(x,16) for x in re.findall(r'0x[0-9a-fA-F]{2}',line)]
                if line_bytes:
                   data = struct.pack(f'<{len(line_bytes)}B', *line_bytes)
                   network += data
                if re.search(r'};',line):
                     break
    return network
            
networks = [get_network(x) for x in args.c]

with open(args.f,"wb") as f:
    offset = 4 # initial offset afterlength of full description (but added at the end)
    network_desc = bytes() # Description of networks in memory
    header = bytes() # 
    header += struct.pack('<II',0xBEEFDEAD,len(networks))
    network_desc += header
    offset += len(header)
    offset += len(networks)*8  # space for table of network description to be filled below

    the_networks = bytes() # Network binaries to add to description
    # Network lengths and offsets
    offsets = []
    for net in networks:
        # align offset
        if offset % ALIGN != 0:
            pad = ALIGN - (offset % ALIGN)
            the_networks += bytes([0]*pad)
            offset += pad
        offsets.append((len(net),offset + args.x))
        the_networks += net 
        offset += len(net)
        
    for len,off in offsets:
        network_desc += struct.pack('<II',len,off)

    network_desc += the_networks
    network_desc = struct.pack('<I',offset) + network_desc
    f.write(network_desc)

    md5 = hashlib.md5(network_desc).hexdigest()
    print(f"md5 hash = {md5}")



