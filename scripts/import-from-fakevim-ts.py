#!/usr/bin/env python3
import sys
import re
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import NamedTuple

class TsItem (NamedTuple):
    name : str
    fullLang : str
    lang : str
    place : str

if len(sys.argv) != 3:
    sys.stderr.write("invalid parameters\n")
    sys.exit(-1)

sourceTsDir = sys.argv[1]
destTsDir = sys.argv[2]

def createTsItem(name):
    fullLang = ""
    lang = ""
    place = ""
    m = re.search("[^_]*_([a-zA-Z_]+)\.ts", name)
    if m:
        fullLang = m[1].lower()
        lang = ""
        place = ""
        if "_" in fullLang:
            lang = fullLang[:fullLang.index("_")]
            place = fullLang[fullLang.index("_")+1:]
        else:
            lang = fullLang
    return TsItem(name=name, fullLang=fullLang, lang=lang, place=place)
destTsItems = []
p = Path(destTsDir)
for f in [x for x in p.glob("encryptpad_*.ts")
        if x.is_file()]:
    destTsItems.append(createTsItem(f.name))

srcTsItems = []
p = Path(sourceTsDir)
for f in [x for x in p.glob("qtcreator_*.ts")
        if x.is_file()]:
    srcTsItems.append(createTsItem(f.name))

matchTuples = []
for dest in destTsItems:
    best = TsItem("", "", "", "")
    for src in (x for x in srcTsItems if dest.lang == x.lang):
        if dest.place == src.place:
            best = src
            break
        best = src
    if best.name:
        matchTuples.append( (best, dest) )
for src, dest in matchTuples:
    print(src, dest)
