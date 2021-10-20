#!/usr/bin/env python3
from PIL import Image
import xml.etree.ElementTree as ET
import sys

if len(sys.argv)<2:
    print('usage: {} <filename.tmx>'.format(sys.argv[0]))
    sys.exit(1)

doc = ET.parse(sys.argv[1])

# assume only one tileset
tileset=doc.findall('tileset')[0]
pitch = tileset.get('columns')
tilesize = tileset.get('tilewidth')
firstgid = int(tileset.get('firstgid'))
print('file={}, pitch={}, tilesize={}'.format(sys.argv[1],pitch,tilesize))

lcnt=0
for elem in doc.findall('layer'):
    w=int(elem.get('width'))
    h=int(elem.get('height'))
    print('Layer {}: {}x{}'.format(lcnt,w,h))
    data = list(map(lambda x:int(x)-firstgid, elem[0].text.split(",")))
    print(str(set(data)))
    im=Image.new("RGBA",(w,h))
    im.putdata(list(map(lambda x: ((x>>24)&0xff,(x>>16)&0xff,(x>>8)&0xff,x&0xff),data)))
    im.save('layer{}.png'.format(lcnt),"PNG")
    lcnt=lcnt+1



