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

if len(sys.argv) != 3:
    sys.stderr.write("invalid parameters\n")
    sys.exit(-1)

sourceTsDir = sys.argv[1]
destTsDir = sys.argv[2]

destTsItems = []
p = Path(destTsDir)
for f in [x for x in p.glob("fakevim_*.ts")
        if x.is_file()]:
    destTsItems.append(createTsItem(f.name))

srcTsItems = []
p = Path(sourceTsDir)
for f in [x for x in p.glob("qtcreator_*.ts")
        if x.is_file()]:
    srcTsItems.append(createTsItem(f.name))

# Match src and dest files

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

# Process FakeVim context elements
def getContextName(context):
    n = context[0]
    assert n.tag == "name"
    return n.text

def getMsgSource(msg):
    for child in msg:
        if child.tag != "source":
            continue
        return child.text
    return None

def findMessage(context, msgSrc):
    for msg in context:
        if msg.tag != "message":
            continue
        srcText = getMsgSource(msg)
        if srcText == msgSrc:
            return msg
    return None

def findTranslation(msg):
    for e in msg:
        if e.tag != "translation":
            continue
        return e
    return None

def updateMessages(srcContext, destContext):
    for destMsg in destContext:
        if destMsg.tag != "message":
            continue
        foundMsgInSrc = findMessage(srcContext, getMsgSource(destMsg))
        if foundMsgInSrc == None:
            continue
        srcTr = findTranslation(foundMsgInSrc)
        assert srcTr != None
        destTr = findTranslation(destMsg)
        assert destTr != None
        destTr.text = srcTr.text
        destMsg.remove(destTr)
        destMsg.append(srcTr)
        print("updated: " + getMsgSource(destMsg))

class EditedTsFile:
    def __init__(self, filePath):
        self.tree = ET.parse(filePath)
        self.root = self.tree.getroot()
    def findContext(self, name):
        for context in self.root:
            if getContextName(context) == name:
                return context
        return None

    def deleteContext(self, name):
        for context in root:
            if getContextName(context) == name:
                self.root.remove(context)
    def addContext(self, context):
        self.root.append(context)
    def dump(self):
        ET.dump(self.root)
    def write(self, path):
        self.tree.write(path, encoding="UTF8")

for src, dest in matchTuples:
    destTs = EditedTsFile(Path(destTsDir) / dest.name)
    p = Path(sourceTsDir) / src.name
    root = ET.parse(p).getroot()
    for srcCtx in root:
        name = getContextName(srcCtx)
        if not name.startswith("FakeVim"):
            continue
        destContext = destTs.findContext("FakeVim")
        if destContext == None:
            continue
        updateMessages(srcCtx, destContext)
    destTs.write(Path("tmp") / dest.name)

