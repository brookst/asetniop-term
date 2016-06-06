// Asetniop terminal program
// Tim Brooks 2014 <brooks@skoorb.net>
//
// Captures scancodes from a linux device node and emits characters according to asetniop mappings
//
// See http://asetniop.com/
// Based on http://www.thelinuxdaily.com/2010/05/grab-raw-keyboard-input-from-event-device-node-devinputevent/
//
// DEBUG FLAGS:
//  * DEBUG - show device aquisition messages
//  * DEBUG_EVENT - show key up and key down events
//  * DEBUG_STATE - show asetniop state during keypresses

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#ifdef DEBUG_EVENT
#define EVENT_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define EVENT_PRINT(fmt, args...)
#endif

#define SHIFT       'V'
#define SPACE       'N'

#define STATE_SHIFT 0x01
#define STATE_SPACE 0x02
#define STATE_FIVE  0x04
#define STATE_SIX   0x08
#define STATE_A     0x80
#define STATE_S     0x40
#define STATE_E     0x20
#define STATE_T     0x10
#define STATE_N     0x08
#define STATE_I     0x04
#define STATE_O     0x02
#define STATE_P     0x01

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

/*[[[cog
import cog
import gen_maps
def cog_stanza(stanza):
    for line in stanza():
        cog.outl(line)

cog_stanza(gen_maps.bits_stanza)
cog.outl("")
cog_stanza(gen_maps.scancode_stanza)
cog.outl("")
cog_stanza(gen_maps.lowercase_stanza)
cog.outl("")
cog_stanza(gen_maps.uppercase_stanza)
cog.outl("")
cog_stanza(gen_maps.numeral_stanza)
cog.outl("")
cog_stanza(gen_maps.symbol_stanza)
]]]*/
//ASETNIOP bits
//A:80 S:40 E:20 T:10 N:08 I:04 O:02 P:01

//Map scan codes to asetniop keys V:Shift N:Space
unsigned char keys_map[256] =
    "  1234567890    aset  niop    aset  niop  V VVVV NNNN V  N      "
    "                                                                "
    "                                                                "
    "        0123456789                                              "
;

//Map asetniop bit patterns to lower-case chars
unsigned char lower_letters_map[256] = {
    0x00, 0x70, 0x6F, 0x3B, 0x69, 0x6B, 0x6C, 0x00,
    0x6E, 0x6D, 0x75, 0x00, 0x68, 0x00, 0x00, 0x00,
    0x74, 0x08, 0x67, 0x00, 0x76, 0x00, 0x00, 0x00,
    0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x65, 0x27, 0x2D, 0x00, 0x2C, 0x00, 0x00, 0x00,
    0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x73, 0x29, 0x2E, 0x00, 0x7A, 0x00, 0x00, 0x00,
    0x6A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x61, 0x3F, 0x28, 0x00, 0x21, 0x00, 0x00, 0x00,
    0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//Map asetniop bit patterns to upper-case chars
unsigned char upper_letters_map[256] = {
    0x00, 0x50, 0x4F, 0x3A, 0x49, 0x4B, 0x4C, 0x00,
    0x4E, 0x4D, 0x55, 0x00, 0x48, 0x00, 0x00, 0x00,
    0x54, 0x7F, 0x47, 0x00, 0x56, 0x00, 0x00, 0x00,
    0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x45, 0x22, 0x5F, 0x00, 0x3C, 0x00, 0x00, 0x00,
    0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x53, 0x1B, 0x3E, 0x00, 0x5A, 0x00, 0x00, 0x00,
    0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x41, 0x2F, 0x1E, 0x00, 0x1E, 0x00, 0x00, 0x00,
    0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//Map asetniop bit patterns to numeral chars
unsigned char numeral_map[256] = {
    0x00, 0x30, 0x39, 0x3B, 0x38, 0x5C, 0x3D, 0x00,
    0x37, 0x5D, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x34, 0x08, 0x13, 0x00, 0x12, 0x00, 0x00, 0x00,
    0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x33, 0x27, 0x2D, 0x00, 0x2C, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x32, 0x29, 0x2E, 0x00, 0x3D, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x31, 0x3F, 0x28, 0x00, 0x21, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//Map asetniop bit patterns to symbol chars
unsigned char symbol_map[256] = {
    0x00, 0x29, 0x28, 0x3A, 0x2A, 0x7C, 0x2B, 0x00,
    0x26, 0x7D, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x24, 0x08, 0x13, 0x00, 0x12, 0x00, 0x00, 0x00,
    0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x23, 0x22, 0x5F, 0x00, 0x3C, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x1B, 0x3E, 0x00, 0x2B, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x21, 0x2F, 0x1E, 0x00, 0x1E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
//[[[end]]]

void handler(int sig) {
    printf("\nexiting...(%d)\n", sig);
    fflush(stdout);
    system("stty echo");
    exit(0);
}

void perror_exit(char *error) {
    perror(error);
    handler(9);
}

struct {
    unsigned char finger_keys;
    unsigned char thumb_keys;
} state;

void zero_state() {
    state.finger_keys = 0;
    state.thumb_keys = 0;
}

void add_state(char a) {
    //[97, 115, 101, 116, 110, 105, 111, 112]
    if(a == SHIFT || a == SPACE) {
        //do nothing
    } else if(a >= '0' && a <= '9') {
        state.thumb_keys |= 0x10;
    } else if(a >= 'a') {
        state.thumb_keys &= ~0x10;
    }
    switch(a) {
        case SHIFT: //Shift
            state.thumb_keys ^= STATE_SHIFT;
            break;
        case SPACE: //Space
            state.thumb_keys |= STATE_SPACE;
            break;
        case '5':
            state.thumb_keys |= STATE_FIVE;
            break;
        case '6':
            state.thumb_keys |= STATE_SIX;
            break;
        case 'a':
        case '1':
            state.finger_keys |= STATE_A;
            break;
        case 's':
        case '2':
            state.finger_keys |= STATE_S;
            break;
        case 'e':
        case '3':
            state.finger_keys |= STATE_E;
            break;
        case 't':
        case '4':
            state.finger_keys |= STATE_T;
            break;
        case 'n':
        case '7':
            state.finger_keys |= STATE_N;
            break;
        case 'i':
        case '8':
            state.finger_keys |= STATE_I;
            break;
        case 'o':
        case '9':
            state.finger_keys |= STATE_O;
            break;
        case 'p':
        case '0':
            state.finger_keys |= STATE_P;
            break;
    }
}

void print_finger_key(int mask, char label) {
    if(state.finger_keys & mask) {
        printf("%c", label);
    } else {
        printf(" ");
    }
}

char* print_state() {
    char * array = malloc(31 * sizeof(char));
    if(array == NULL) {
        printf("Fail!\n");
        exit(1);
    }
    array[0] = (state.finger_keys & 0x80) ? 'a' : 32;
    array[1] = (state.finger_keys & 0x40) ? 's' : 32;
    array[2] = (state.finger_keys & 0x20) ? 'e' : 32;
    array[3] = (state.finger_keys & 0x10) ? 't' : 32;
    array[4] = (state.finger_keys & 0x08) ? 'n' : 32;
    array[5] = (state.finger_keys & 0x04) ? 'i' : 32;
    array[6] = (state.finger_keys & 0x02) ? 'o' : 32;
    array[7] = (state.finger_keys & 0x01) ? 'p' : 32;
    sprintf(array +  8, "%1s", (state.thumb_keys & 0x04) ? "5" : "");
    sprintf(array +  9, "%1s", (state.thumb_keys & 0x08) ? "6" : "");
    sprintf(array + 10, "%6s", (state.thumb_keys & 0x01) ? "SHIFT" : "");
    sprintf(array + 16, "%6s", (state.thumb_keys & 0x02) ? "SPACE" : "");
    sprintf(array + 22, "%8s", (state.thumb_keys & 0x10) ? "NUMERIC" : "");
    array[30] = 0;  //NULL
    return array;
}

char get_char() {
    if(state.thumb_keys == 0x3 && state.finger_keys == 0x0) {
        return '\n';
    } else if(state.thumb_keys == 0x2 && state.finger_keys == 0x0) {
        return ' ';
    } else if(state.finger_keys == 0x11) {
        return -1;
    } else if(state.thumb_keys == 0x0) {
        return lower_letters_map[state.finger_keys];
    } else if(state.thumb_keys == 0x1) {
        return upper_letters_map[state.finger_keys];
    } else if(state.thumb_keys == 0x10) {
        return numeral_map[state.finger_keys];
    } else if(state.thumb_keys == 0x11) {
        return symbol_map[state.finger_keys];
    } else if(state.thumb_keys == 0x14) {
        return '5';
    } else if(state.thumb_keys == 0x15) {
        return '%';
    } else if(state.thumb_keys == 0x18) {
        return '6';
    } else if(state.thumb_keys == 0x19) {
        return '^';
    } else if(state.thumb_keys == 0x1c) {
        return '\n';
    } else {
        return 0x0;
    }
}

int main(int argc, char *argv[]) {
    struct input_event ev[64];
    int fd, rd, size = sizeof(struct input_event);
    char name[256] = "Unknown";
    char *device = NULL;
    zero_state();

    //Setup check
    if(argv[1] == NULL){
        fputs("Please specify (on the command line) the path to the device event interface\n", stderr);
        exit(0);
    }

    if((getuid()) != 0)
        fputs("You are not root! Ensure you have read permissions on this device\n", stderr);

    if(argc > 1)
        device = argv[1];

    signal(SIGINT, handler);

    //Open Device
    if((fd = open(device, O_RDONLY)) == -1)
        fprintf(stderr, "%s is not a valid device.\n", device);

    //Print Device Name
    ioctl(fd, EVIOCGNAME (sizeof(name)), name);
    DEBUG_PRINT("Reading from: %s (%s)\n", device, name);
    system("stty -echo");

    while(1) {
        if((rd = read(fd, ev, size * 64)) < size)
            perror_exit("read()");

        if(ev[1].type == 1){ // Only read the key press event
            if(ev[1].value == 1) {
                EVENT_PRINT("Keydown [%d]: %c\n", (ev[1].code), keys_map[ev[1].code]);
                add_state(keys_map[ev[1].code]);
#if DEBUG_STATE
                char * msg = print_state();
                printf("\n");
                fputs(msg, stderr);
                free(msg);
#endif
            } else if(ev[1].value == 0) {
                EVENT_PRINT("Keyup   [%d]: %c\n", (ev[1].code), keys_map[ev[1].code]);
                if(keys_map[ev[1].code] == SHIFT) { //Bail out if this is a shift release
                    continue;
                }
                char c;
                if((c = get_char()) > 0) {
#if DEBUG_STATE
                    fputs(": ", stderr);
#endif
                    printf("%c", c);
                    fflush(stdout);
                } else if(c == -1) {
                    printf("\b \b");
                    fflush(stdout);
                }
                zero_state();
            }
        }
    }

    return 0;
}
