#! /usr/bin/env python

import sys
infilename = sys.argv[1]

f = open(infilename)
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

outfilename = infilename.replace('.n42', '.txt')
data = get_histogram_data(f)
write_histogram_data(outfilename, data)
