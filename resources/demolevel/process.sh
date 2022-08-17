#!/usr/bin/bash
./parse_tmx.py terrain.tmx | sed -e 's/%T%//g' | sed -e 's/%R%/DemoLevelResources/g' > ../../src/demolevel.inc
