#!/usr/bin/python

# Copyright 2013-present Barefoot Networks, Inc. 
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from scapy.all import sendp, send, srp1
from scapy.all import sniff, sendp 
from scapy.all import Packet
from scapy.all import ShortField, IntField, LongField, BitField, ByteField


from scapy.all import bind_layers, Ether

import networkx as nx

import sys

class KeyValue(Packet):
    name = "KeyValue"
    fields_desc = [
        LongField("preamble", 1), #set to 1
        IntField("num_valid", 1),
        ByteField("port", 0),
        ByteField("mtype", 0),
        IntField("key", 0),
        IntField("value", 0),
    ]


def read_topo():
    nb_hosts = 0
    nb_switches = 0
    links = []
    with open("topo.txt", "r") as f:
        line = f.readline()[:-1]
        w, nb_switches = line.split()
        assert(w == "switches")
        line = f.readline()[:-1]
        w, nb_hosts = line.split()
        assert(w == "hosts")
        for line in f:
            if not f: break
            a, b = line.split()
            links.append( (a, b) )
    return int(nb_hosts), int(nb_switches), links



def main():

    #read argvs
    if len(sys.argv) !=3 and len(sys.argv) != 4:
        print "Usage: kv.py put [key] [value]/ get [key]"
        print "For example: kv.py put 1 11 "
        print "For example: kv.py get 1"
        sys.exit(1)

    if len(sys.argv) == 3 and sys.argv[1] == 'get': #get
        key = int(sys.argv[2])
        query_type = 0
        val = 0 #default value?     
    elif len(sys.argv) == 4 and sys.argv[1] == 'put': #put
        key = int(sys.argv[2])
        val = int(sys.argv[3])
        query_type = 1

    p = KeyValue(port = 1, mtype= query_type, key = key, value = val) 
    print "after creating p"
    print p.show()
    sendp(p, iface = "eth0")

if __name__ == '__main__':
    main()