#include <absacc.h>
#include <ctype.h>
#include <reg51.h>

void DispClear();
void DispChar(unsigned char x, unsigned char c, bit dp);
void DispStr(unsigned char x, unsigned char idata* s);
void numToStr(unsigned char idata* s, unsigned long c);
void DispInit();
void SysInit();
sbit led = P0 ^ 7;

unsigned int cnt0 = 0;
unsigned char DispBuf[8];
unsigned char cnt0_lowl; 
unsigned char cnt0_lo = 0;
unsigned char cnt0_hi = 0;
unsigned char cnt0_overflow = 0;

volatile bit flag;

void T1INTSVC() interrupt 3 {
    code unsigned char com[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    static unsigned char n = 0;
    TR1 = 0;
    TH1 = 0xFA;
    TL1 = 0x27;
    TR1 = 1;
    XBYTE[0x7800] = 0xFF;
    XBYTE[0x7801] = ~DispBuf[n];
    XBYTE[0x7800] = ~com[n];
    n++;
    n &= 0x07;
    flag = 1;
}


void T0INTSVC() interrupt 1 { cnt0++; }

void DispClear() {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        DispBuf[i] = 0x00;
    }
}

void DispChar(unsigned char x, unsigned char c, bit dp) {
    code unsigned char Tab[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                                0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    unsigned char t;
    x &= 0x07;
    x = 7 - x;
    if (c == '-') {
        t = 0x40;
    } else {
        t = toint(c);
        if (t < 16) {
            t = Tab[t];
        } else {
            t = 0x00;
        }
    }
    if (dp) {
        t |= 0x80;
    } else {
        t &= 0x7F;
    }
    DispBuf[x] = t;
}

void DispStr(unsigned char x, unsigned char idata* s) {
    unsigned char c;
    for (;;) {
        c = *s++;
        if (c == '\0') break;
        DispChar(x++, c, 0);
    }
}


void numToStr(unsigned char idata* s, unsigned long c) {
    code unsigned long Tab[] = {100000, 10000, 1000, 100, 10};
    unsigned char i;
    unsigned char t;
    for (i = 0; i < 5; i++) {
        t = c / Tab[i];
        *s++ = '0' + t;
        c -= t * Tab[i];
    }
    *s++ = '0' + c;
    *s = '\0';
}

void DispInit() {
    DispClear();
    EA = 0;
    TMOD &= 0x0F;   /* T1 mode clear */
    TMOD |= 0x10;   /* Timer1 mode1 */
    TH1 = 0xFC;
    TL1 = 0x66;
    TR1 = 1;
    ET1 = 1;
    EA = 1;
}

void SysInit() {
    TMOD &= 0xF0;

    TMOD |= 0x05;
    TR0 = 0;
    ET0 = 1;
    DispInit();
}

void main() {
    unsigned int time = 0;
    unsigned char idata s[7];
    unsigned long frequency;

    SysInit();


    cnt0 = 0;
    TH0 = 0;
    TL0 = 0;

    for (;;) {
        flag = 0;
        while (!flag);
        time++;


        if (time == 100) {
 
            cnt0 = 0;
            TH0 = 0;
            TL0 = 0;
          
            TR0 = 1;
            led = 0;
        }

        if (time == 1100) {
            TR0 = 0;   
            led = 1;


            frequency = ((unsigned long)cnt0) * 65536
                        + ((unsigned long)TH0 << 8)
                        + (unsigned long)TL0;

			if(frequency < 100000) frequency = frequency;
			else if(frequency < 200000) frequency = frequency - 1;
			else if(frequency < 300000) frequency = frequency - 3;
			else if(frequency < 400000) frequency = frequency - 5;
			else if(frequency < 500000) frequency = frequency - 7;
			else if(frequency < 600000) frequency = frequency - 9;
			else if(frequency < 700000) frequency = frequency - 10;
			else frequency = frequency - 11;


            numToStr(s, frequency);
            DispStr(1, s);


            DispBuf[4] |= 0x80;


            cnt0 = 0;
            TH0 = 0;
            TL0 = 0;
            time = 0;
        }
    }
}
