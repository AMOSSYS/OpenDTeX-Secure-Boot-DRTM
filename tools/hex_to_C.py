import sys
import struct

if len(sys.argv) != 1:
    print "Usage: python %s" % sys.argv[0]
    exit(0)

# Read source
sfd = open("image_struct.data")
sdata = sfd.read()
sfd.close()

# Retrieve size and offset of raw bitmap data
#(typeD,) = struct.unpack("<I", sdata[0:4])
#(dataSize,) = struct.unpack("<I", sdata[4:8])
dataOffset = 0
dataSize = len(sdata)

structC = "BYTE ext_data[] = {"
for i in range(dataSize):
    if i % 16 == 0:
        structC += "\n"
    structC += "0x" + sdata[dataOffset + i].encode("hex") + ", "
structC = structC[:-2] #On enleve la derniere virgule...
structC += "\n};"

print structC
