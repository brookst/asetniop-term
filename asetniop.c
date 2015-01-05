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
]]]*/
//ASETNIOP bits
//A:80 S:40 E:20 T:10 N:08 I:04 O:02 P:01

//Map scan codes to asetniop keys
unsigned char keys_map[256] =
    "                aset  niop    aset  niop  1 1111 2222 1  2      "
    "                                                                "
    "                                                                "
    "                                                                "
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
    0x54, 0x08, 0x47, 0x00, 0x56, 0x00, 0x00, 0x00,
    0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x45, 0x22, 0x5F, 0x00, 0x3C, 0x00, 0x00, 0x00,
    0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x53, 0x00, 0x3E, 0x00, 0x5A, 0x00, 0x00, 0x00,
    0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x41, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
//[[[end]]]

void handler(int sig) {
    printf("\nexiting...(%d)\n", sig);
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

void add_state(unsigned char a) {
    //[97, 115, 101, 116, 110, 105, 111, 112]
    switch(a) {
        case '1': //Shift
            state.thumb_keys ^= 0x01;
            break;
        case '2': //Space
            state.thumb_keys |= 0x02;
            break;
        case 'a':
            state.finger_keys |= 0x80;
            break;
        case 's':
            state.finger_keys |= 0x40;
            break;
        case 'e':
            state.finger_keys |= 0x20;
            break;
        case 't':
            state.finger_keys |= 0x10;
            break;
        case 'n':
            state.finger_keys |= 0x08;
            break;
        case 'i':
            state.finger_keys |= 0x04;
            break;
        case 'o':
            state.finger_keys |= 0x02;
            break;
        case 'p':
            state.finger_keys |= 0x01;
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
    char * array = malloc(21 * sizeof(char));
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
    sprintf(array + 8, " %s", (state.thumb_keys & 0x1) ? "SHIFT" : "     ");
    sprintf(array + 14, " %s", (state.thumb_keys & 0x2) ? "SPACE" : "     ");
    array[20] = 0;  //NULL
    return array;
}

unsigned char get_char() {
    if(state.thumb_keys == 0x3 && state.finger_keys == 0x0) {
        return '\n';
    } else if(state.thumb_keys == 0x2 && state.finger_keys == 0x0) {
        return ' ';
    } else if(state.thumb_keys == 0x0) {
        return lower_letters_map[state.finger_keys];
    } else if(state.thumb_keys == 0x1) {
        return upper_letters_map[state.finger_keys];
    } else {
        return 0x0;
    }
}

int main(int argc, char *argv[]) {
    struct input_event ev[64];
    int fd, rd, value, size = sizeof(struct input_event);
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

    //Open Device
    if((fd = open(device, O_RDONLY)) == -1)
        fprintf(stderr, "%s is not a vaild device.\n", device);

    //Print Device Name
    ioctl(fd, EVIOCGNAME (sizeof(name)), name);
    DEBUG_PRINT("Reading from: %s (%s)\n", device, name);
    system("stty -echo");

    while(1) {
        if((rd = read(fd, ev, size * 64)) < size)
            perror_exit("read()");

        value = ev[0].value;

        // if(value != ' ' && ev[1].value == 1 && ev[1].type == 1){ // Only read the key press event
        if(value != ' ' && ev[1].type == 1){ // Only read the key press event
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
                if(keys_map[ev[1].code] == '1') { //Bail out if this is a shift release
                    continue;
                }
                unsigned char c;
                if((c = get_char())) {
#if DEBUG_STATE
                    fputs(": ", stderr);
#endif
                    printf("%c", c);
                    fflush(stdout);
                }
                zero_state();
            }
        }
    }

    return 0;
}
