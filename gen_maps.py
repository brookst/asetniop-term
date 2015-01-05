#!/usr/bin/env python
"""Generate asetniop mappings"""

from __future__ import print_function
from itertools import combinations_with_replacement
from string import ascii_lowercase

#The base keys
ASETNIOP_KEYS = "asetniop"
#Bit locations of the base keys
ASETNIOP_BITS = {c: 1 << (7-i) for i, c in enumerate(ASETNIOP_KEYS)}
#ASCII codes to emit in response to chords
ASETNIOP_CHARS = {"aa": "a", "as": "w", "ae": "x", "at": "f",
                  "an": "q", "ai": "!", "ao": "(", "ap": "?",
                  "ss": "s", "se": "d", "st": "c",
                  "sn": "j", "si": "z", "so": ".", "sp": ")",
                  "ee": "e", "et": "r",
                  "en": "y", "ei": ",", "eo": "-", "ep": "'",
                  "tt": "t",
                  "tn": "b", "ti": "v", "to": "g", "tp": "\b",
                  "nn": "n", "ni": "h", "no": "u", "np": "m",
                  "ii": "i", "io": "l", "ip": "k",
                  "oo": "o", "op": ";",
                  "pp": "p"}
#Translation into upper case
ASETNIOP_SHIFT = {c: c.upper() for c in ascii_lowercase}
ASETNIOP_SHIFT["!"] = "\0" #CTRL
ASETNIOP_SHIFT["("] = "\0" #ALT
ASETNIOP_SHIFT["?"] = "/"
ASETNIOP_SHIFT["."] = ">"
ASETNIOP_SHIFT[")"] = "\0" #ESC
ASETNIOP_SHIFT[","] = "<"
ASETNIOP_SHIFT["-"] = "_"
ASETNIOP_SHIFT["'"] = "\""
ASETNIOP_SHIFT["\b"] = "\b" #DEL
ASETNIOP_SHIFT[";"] = ":"

#Scan-codes for a Microsoft Comfort Curve Keyboard
#1: Shift key, 2: Space key
SCANCODE_ASETNIOP = "          " "      aset" "  niop    " "aset  niop" \
                    "  1 1111 2" "222 1  2  " "          " "          " \
                    "          " "          " "          " "          " \
                    "          " "          " "          " "          " \
                    "          " "          " "          " "          " \
                    "          " "          " "          " "          " \
                    "          " "      "

def chunks(seq, length):
    """Chunk seq into length sized sub-sequences"""
    for index in range(0, len(seq), length):
        yield seq[index:index+length]

LOWER_LETTERS_MAP = ["\0"] * 256
UPPER_LETTERS_MAP = ["\0"] * 256
for key in list(combinations_with_replacement(ASETNIOP_KEYS, 2)):
    bits = ASETNIOP_BITS[key[0]] | ASETNIOP_BITS[key[1]]
    char = ASETNIOP_CHARS[key[0] + key[1]]
    LOWER_LETTERS_MAP[bits] = char
    UPPER_LETTERS_MAP[bits] = ASETNIOP_SHIFT[char]

def bits_stanza():
    """ASETNIOP bits"""
    ret = []
    ret.append("//" + bits_stanza.__doc__)
    ret.append("//" + " ".join("%s:%02X" %(c.upper(), ASETNIOP_BITS[c])
                               for c in ASETNIOP_KEYS))
    return ret

def scancode_stanza():
    """Map scan codes to asetniop keys"""
    ret = []
    ret.append("//" + scancode_stanza.__doc__)
    ret.append("unsigned char keys_map[256] =")
    for chunk in chunks(SCANCODE_ASETNIOP, 64):
        ret.append("    \"%s\"" %chunk)
    ret.append(";")
    return ret

def lowercase_stanza():
    """Map asetniop bit patterns to lower-case chars"""
    ret = []
    ret.append("//" + lowercase_stanza.__doc__)
    ret.append("unsigned char lower_letters_map[256] = {")
    for chunk in chunks(LOWER_LETTERS_MAP, 8):
        ret.append("    " + ", ".join("0x%02X" %ord(c) for c in chunk) + ",")
    ret.append("};")
    return ret

def uppercase_stanza():
    """Map asetniop bit patterns to upper-case chars"""
    ret = []
    ret.append("//" + uppercase_stanza.__doc__)
    ret.append("unsigned char upper_letters_map[256] = {")
    for chunk in chunks(UPPER_LETTERS_MAP, 8):
        ret.append("    " + ", ".join("0x%02X" %ord(c) for c in chunk) + ",")
    ret.append("};")
    return ret

def print_stanza(stanza):
    """Print stanza to stdout"""
    for line in stanza():
        print(line)

def main():
    """Print all maps"""
    print_stanza(bits_stanza)
    print("")
    print_stanza(scancode_stanza)
    print("")
    print_stanza(uppercase_stanza)
    print("")
    print_stanza(uppercase_stanza)

if __name__ == '__main__':
    main()
