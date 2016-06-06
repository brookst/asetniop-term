#!/usr/bin/env python
"""Generate asetniop mappings"""

from __future__ import print_function
from itertools import combinations_with_replacement
from string import ascii_lowercase

#The base keys
ASETNIOP_KEYS = "asetniop"
NUMERAL_KEYS = "12347890"
#Bit locations of the base keys
ASETNIOP_BITS = {c: 1 << (7-i) for i, c in enumerate(ASETNIOP_KEYS)}
NUMERAL_BITS = {c: 1 << (7-i) for i, c in enumerate(NUMERAL_KEYS)}
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

KEY_CTRL = "\x1e"
KEY_ALT = "\x1e"
KEY_ESC = "\x1b"
KEY_DEL = "\x7f"

KEY_SHIFT = "\x0f"
KEY_FN = "\x0e"
KEY_PGUP = "\x01"
KEY_HOME = "\x02"
KEY_END = "\x03"
KEY_PGDOWN = "\x04"
KEY_LEFT = "\x11"
KEY_UP = "\x12"
KEY_RIGHT = "\x13"
KEY_DOWN = "\x14"

#Translation into upper case
ASETNIOP_SHIFT = {c: c.upper() for c in ascii_lowercase}
ASETNIOP_SHIFT["!"] = KEY_CTRL
ASETNIOP_SHIFT["("] = KEY_ALT
ASETNIOP_SHIFT["?"] = "/"
ASETNIOP_SHIFT["."] = ">"
ASETNIOP_SHIFT[")"] = KEY_ESC
ASETNIOP_SHIFT[","] = "<"
ASETNIOP_SHIFT["-"] = "_"
ASETNIOP_SHIFT["'"] = "\""
ASETNIOP_SHIFT["\b"] = KEY_DEL
ASETNIOP_SHIFT[";"] = ":"

NUMERAL_KEYS = "12347890"
NUMERAL_BITS = {c: 1 << (7-i) for i, c in enumerate(NUMERAL_KEYS)}

NUMERAL_CHARS = {"11": "1", "12": KEY_SHIFT, "13": "`", "14": "[",
                 "17": "\x00", "18": "!", "19": "(", "10": "?",
                 "22": "2", "23": "-", "24": KEY_FN,
                 "27": KEY_END, "28": "=", "29": ".", "20": ")",
                 "33": "3", "34": KEY_PGUP,
                 "37": KEY_HOME, "38": ",", "39": "-", "30": "'",
                 "44": "4",
                 "47": KEY_LEFT, "48": KEY_UP, "49": KEY_RIGHT, "40": "\b",
                 "77": "7", "78": KEY_PGDOWN, "79": KEY_FN, "70": "]",
                 "88": "8", "89": "=", "80": "\\",
                 "99": "9", "90": ";",
                 "00": "0"}

SYMBOL_CHARS = {"11": "!", "12": KEY_SHIFT, "13": "~", "14": "{",
                "17": "\x00", "18": KEY_CTRL, "19": KEY_ALT, "10": "/",
                "22": "@", "23": "_", "24": KEY_FN,
                "27": KEY_END, "28": "+", "29": ">", "20": "\x1b",
                "33": "#", "34": KEY_PGUP,
                "37": KEY_HOME, "38": "<", "39": "_", "30": "\"",
                "44": "$",
                "47": KEY_LEFT, "48": KEY_UP, "49": KEY_RIGHT, "40": "\b",
                "77": "&", "78": KEY_PGDOWN, "79": KEY_FN, "70": "}",
                "88": "*", "89": "+", "80": "|",
                "99": "(", "90": ":",
                "00": ")"}

#Scan-codes from include/linux/input.h
#V: Shift key, N: Space key
SCANCODE_ASETNIOP = "  12345678" "90    aset" "  niop    " "aset  niop" \
                    "  V VVVV N" "NNN V  N  " "          " "          " \
                    "          " "          " "          " "          " \
                    "          " "          " "          " "          " \
                    "          " "          " "          " "          " \
                    "0123456789" "          " "          " "          " \
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

NUMERAL_MAP = ["\0"] * 256
SYMBOL_MAP = ["\0"] * 256
for key in list(combinations_with_replacement("12347890", 2)):
    bits = NUMERAL_BITS[key[0]] | NUMERAL_BITS[key[1]]
    char = NUMERAL_CHARS[key[0] + key[1]]
    shift = NUMERAL_CHARS[key[0] + key[1]]
    NUMERAL_MAP[bits] = NUMERAL_CHARS[key[0] + key[1]]
    SYMBOL_MAP[bits] = SYMBOL_CHARS[key[0] + key[1]]

def bits_stanza():
    """ASETNIOP bits"""
    ret = []
    ret.append("//" + bits_stanza.__doc__)
    ret.append("//" + " ".join("%s:%02X" %(c.upper(), ASETNIOP_BITS[c])
                               for c in ASETNIOP_KEYS))
    return ret

def scancode_stanza():
    """Map scan codes to asetniop keys V:Shift N:Space"""
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

def numeral_stanza():
    """Map asetniop bit patterns to numeral chars"""
    ret = []
    ret.append("//" + numeral_stanza.__doc__)
    ret.append("unsigned char numeral_map[256] = {")
    for chunk in chunks(NUMERAL_MAP, 8):
        ret.append("    " + ", ".join("0x%02X" %ord(c) for c in chunk) + ",")
    ret.append("};")
    return ret

def symbol_stanza():
    """Map asetniop bit patterns to symbol chars"""
    ret = []
    ret.append("//" + symbol_stanza.__doc__)
    ret.append("unsigned char symbol_map[256] = {")
    for chunk in chunks(SYMBOL_MAP, 8):
        ret.append("    " + ", ".join("0x%02X" %ord(c) for c in chunk) + ",")
    ret.append("};")
    return ret

def print_stanza(stanza):
    """Print stanza to stdout"""
    for line in stanza():
        print(line)

def main():
    """Print all maps"""
    for stanza in [bits_stanza, scancode_stanza,
                   lowercase_stanza, uppercase_stanza,
                   numeral_stanza, symbol_stanza,
                  ]:
        print_stanza(stanza)
        print("")

if __name__ == '__main__':
    main()
