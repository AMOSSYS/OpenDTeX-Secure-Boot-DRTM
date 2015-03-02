#!/usr/bin/python

from struct import *
import binascii
import sys
import os
import hashlib

if len(sys.argv) != 3:
   raise BaseException("Syntex error: pcrsimul.py <PCR> <Measure>")

if sys.argv[1] == '0':
   sys.argv[1] = '0000000000000000000000000000000000000000'

v = binascii.a2b_hex(sys.argv[1])
m = binascii.a2b_hex(sys.argv[2])

if len(v) != 20:
   raise BaseException("PCR is not a hash value")

if len(m) != 20:
   raise BaseException("measure is not a hash value")
   
hash = hashlib.sha1()
hash.update(v)
hash.update(m)

h = hash.hexdigest()

print h
sys.exit(0)

