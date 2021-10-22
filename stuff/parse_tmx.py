#!/usr/bin/env python3
from PIL import Image
import xml.etree.ElementTree as ET
import sys

if len(sys.argv)<2:
    print('usage: {} <in.tmx> <out.tmx>'.format(sys.argv[0]))
    sys.exit(1)

doc = ET.parse(sys.argv[1])

# assume only one tileset
tileset=doc.findall('tileset')[0]
pitch = tileset.get('columns')
tilesize = tileset.get('tilewidth')
firstgid = int(tileset.get('firstgid'))
print('file={}, pitch={}, tilesize={}'.format(sys.argv[1],pitch,tilesize))


data=[]
w=0
h=0

def offs(x,y):
    return y*w+x

def N(pos):
    res=[-1]*9
    (y,x)=divmod(pos,w)
    for di in range(3):
        for dj in range(3):
            yy=y+di-1
            xx=x+dj-1
            pp=offs(xx,yy)
            if pp<0 or pp>=len(data):
                continue
            res[di*3+dj]=data[pp]
    return res

#rules for sample_level.tmx

l=17 # sand
o=21 # water
x=176 # grass
_=-1
rules=[
# water-sand
[[o,o,_,  o,l,_,  _,_,_], 0],
[[_,o,o,  _,l,o,  _,_,_], 2],
[[_,_,_,  o,l,_,  o,o,_], 32],
[[_,_,_,  _,l,o,  _,o,o], 34],

[[_,_,_,  _,l,l,  _,l,o], 4],
[[_,_,_,  l,l,_,  o,l,_], 6],
[[_,l,o,  _,l,l,  _,_,_], 36],
[[o,l,_,  l,l,_,  _,_,_], 38],

[[_,o,_,  _,l,_,  _,l,_], 1],
[[_,_,_,  o,l,l,  _,_,_], 16],
[[_,_,_,  l,l,o,  _,_,_], 18],
[[_,l,_,  _,l,_,  _,o,_], 33],

#sand-grass
[[_,_,_,  _,x,x,  _,x,l], 192],
[[_,_,_,  x,x,_,  l,x,_], 194],
[[_,x,l,  _,x,x,  _,_,_], 224],
[[l,x,_,  x,x,_,  _,_,_], 226],

[[l,l,_,  l,x,_,  _,_,_], 195],
[[_,l,l,  _,x,l,  _,_,_], 197],
[[_,_,_,  l,x,_,  l,l,_], 227],
[[_,_,_,  _,x,l,  _,l,l], 229],

[[_,x,_,  _,x,_,  _,l,_], 193],
[[_,_,_,  x,x,l,  _,_,_], 208],
[[_,_,_,  l,x,x,  _,_,_], 210],
[[_,l,_,  _,x,_,  _,x,_], 225],
]


# don't use rules
#rules=[]

def match(a,b):
    for i in range(len(a)):
        if a[i]>-1 and a[i]!=b[i]:
            return False
    return True

def apply_rule(x):
    for r in rules:
        if match(r[0],x):
            return r[1]
    return x[4]

def show_n(x,y):
    print(N(offs(x,y)))

lcnt=0
for elem in doc.findall('layer'):
    w=int(elem.get('width'))
    h=int(elem.get('height'))
    print('Layer {}: {}x{}'.format(lcnt,w,h))
    data = list(map(lambda x:int(x)-firstgid, elem[0].text.split(",")))
    print(str(set(data)))
    print(rules)
    res=[0]*len(data)
    for i in range(len(data)):
        res[i]=apply_rule(N(i))+firstgid
    elem[0].text=str(res)[1:-1]
    im=Image.new("RGBA",(w,h))
    im.putdata(list(map(lambda x: ((x>>24)&0xff,(x>>16)&0xff,(x>>8)&0xff,x&0xff),data)))
    im.save('layer{}.png'.format(lcnt),"PNG")
    lcnt=lcnt+1

if len(sys.argv)>2:
    doc.write(sys.argv[2])


