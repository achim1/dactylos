#! /usr/bin/env python

import re
import os
import os.path

from glob import glob

def stripname(ch):
    return "ABCDEFGH"[ch]

def rename_folders():
    pattern = re.compile("(?P<pt>[0-9]*)ns")
    folders = glob("*ns")
    for f in folders:
        print (f)
        data = pattern.search(f)
        print (data)
        newname = f'pt_{data.groupdict()["pt"]}_ns'
        print (newname)
        os.rename(f, newname)

def convert_files():
    folders = glob("pt*ns")
    for folder in folders:
        files = glob(os.path.join(folder, "*.n42"))
        for f in files:
            hd = get_histogram_data(open(f,'r'))
            ch = int(f.split(".")[0][-1])
            strip = stripname(ch)
            newname = f"Sh235_{strip}.txt"
            newname = os.path.join(folder, newname)
            print (f'{f} -> {newname}')
            write_histogram_data(newname, hd)            
            #os.rename(f, newname)            
        #path = os.path.split(name)[0]
        #print (path)

def get_histogram_data(f):
    start = False
    data = []
    for line in f.readlines():
        if line.lstrip().startswith('</ChannelData>'):
            start = False
        
        if line.lstrip().startswith('<ChannelData compressionCode="None">'):
            nchar = len('<ChannelData compressionCode="None">')
            line = line.lstrip()
            line = line[nchar:]
            print (line)
            line = line.split()
            for k in line:
                data.append(int(k))
            start = True
        elif start:
            line = line.split()
            for k in line:
                data.append(int(k))

    return data

def write_histogram_data(filename, data):
    f = open(filename,'w')
    for k in data:
        f.write(f'{k}\n')
    f.flush()
    f.close()

def get_strip(ch):
    CHANNELS = "ABCDEFGH"
    return CHANNELS[ch]

if __name__ == '__main__':
    #rename_files()
    #rename_folders()
    rename_folders()
    convert_files()
