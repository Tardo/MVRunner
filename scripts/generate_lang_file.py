#!/usr/bin/python
# Alexandre Díaz - 2018 - GPL
# Generate language file
#
# Working Directory: Project Dir
# Usage:
#   generate_lang_file.py mylang.txt
#
from os import walk
from os.path import join
import re
import sys

def _analize_file(fname):
    with open(fname) as f:
        content = f.readlines()
    
    translate = []
    for line in content:
        matches = re.findall(r'_\("([^"]+)"\)', line)
        translate.extend(matches)
    return translate

def _analize_dir(path):
    files = []
    for (dirpath, dirnames, filenames) in walk(path):
        filenames = [join(dirpath, x) for x in filenames]
        files.extend(filenames)
    
    toTranslate = []
    for sfile in files:
        toTranslate.extend(_analize_file(sfile))
    return toTranslate
    
def create_lang_file(fname):
    with open(fname, "w") as output:
        for phrase in _analize_dir('src/'):
            output.write("%s=\n" % phrase)


if __name__ == '__main__':
    create_lang_file(sys.argv[1])
