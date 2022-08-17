#!/usr/bin/env python3

# create tilemaps and print initialization code
# expect tmx with csv and embedded tilesets
# by default tiles are passable, non-passable tiles must have set "passable":false

from PIL import Image
import xml.etree.ElementTree as ET
import sys
import os

if len(sys.argv)<2:
    print('usage: {} <in.tmx>'.format(sys.argv[0]))
    sys.exit(1)

doc = ET.parse(sys.argv[1])

gids=set()
for data in doc.findall('.//layer/data'):
    gids= gids.union(set(map(lambda x : int(x), data.text.split(','))))

class TileSheet(): 
    __slots__ = 'source','firstgid','columns','tilewidth','tileheight','tiles','solid'
    def __init__(self,s,g,c,tw,th):
        self.source=s
        self.firstgid=g
        self.columns=c
        self.tilewidth=tw
        self.tileheight=th
        self.tiles=[]
        self.solid=[] 

tilesets=[]
for tileset in doc.findall('.//tileset'):
    tilesets.append(TileSheet(tileset.find('./image').get('source'),
        int(tileset.get('firstgid')),int(tileset.get('columns')),
        int(tileset.get('tilewidth')),int(tileset.get('tileheight'))))
    for t in tileset.findall('.//tile'):
        pid=int(t.get('id'))
        p=t.findall('.//property[@name="passable"]')
        if len(p)>0 and p[0].get('value')=="false":
            tilesets[-1].solid.append(pid)

    
tilesets.append(TileSheet("empty.png",0,1,tilesets[0].tilewidth,tilesets[0].tileheight))    
tilesets=sorted(tilesets,key=lambda x:-x.firstgid)


for gid in gids:
    next(filter(lambda x:x.firstgid<=gid, tilesets)).tiles.append(gid)

print("// replace %T% with TerrainMap, %R% with resource namespace\n")
for t in filter(lambda x:len(x.tiles)>0,tilesets):
    s = os.path.splitext(os.path.basename(t.source))[0]
    t.source=s
    print("%T%addTileSheet(\"{}\",%R%::{}(),{},{},{});".format(
        s,s,
        t.columns,t.tilewidth,t.tileheight))

print()
for t in filter(lambda x:len(x.tiles)>0,tilesets):
    ids=list(map(lambda x:x-t.firstgid,t.tiles))
    print("%T%addTiles(\"{}\",{{{}}},{{{}}},{{{}}});".format(
        t.source,str(t.tiles)[1:-1],str(ids)[1:-1],str(t.solid)[1:-1]))

print()
lcnt=0
for layer in doc.findall('.//layer'):
    w=int(layer.get('width'))
    h=int(layer.get('height'))
    op=layer.get('opacity')
    if op!=None:
        opacity=float(op)
    else:
        opacity=1.0
    data = list(map(lambda x:int(x), layer[0].text.split(",")))
    im=Image.new("RGBA",(w,h))
    im.putdata(list(map(lambda x: ((x>>24)&0xff,(x>>16)&0xff,(x>>8)&0xff,x&0xff),data)))
    im.save('layer{}.png'.format(layer.get('id')),"PNG")
    print("%T%addTileMap({},%R%::layer{}(),{});".format(lcnt,layer.get('id'),opacity))
    lcnt=lcnt+1


