#include "rpiPWM1.h"
#include <iostream>
#include <unistd.h> 
#include <sys/time.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}


int main (void){
	    
    double val = 8.0;    
    char key = ' ';
    set_conio_terminal_mode();
	
    rpiPWM1 pwm(50.0, 1024, val, rpiPWM1::MSMODE);
    // initialize PWM1 output to 1KHz 8-bit resolution 80% Duty Cycle & PWM mode is MSMODE    
	printf("%3.2lf Duty Cycle is %3.2lf \n", val, pwm.getDutyCycle());
	
    while (key != 'q')
    {
		if(!kbhit())
		{
			key = getch();
			switch (key) {
				case 'y':
				val += 1.0;
				break;				
				case 'h':
				val -= 1.0;
				break;
				
				case 'u':
				val += 0.1;
				break;				
				case 'j':
				val -= 0.1;
				break;
				
				case 'i':
				val += 0.4;
				break;				
				case 'k':
				val -= 0.4;
				break;
				
				case 't':
				val = 12.8;
				break;				
				case 'g':
				val = 3.6;
				break;
				
				default:
				break;
			}				
				
			pwm.setDutyCycle(val);
			printf("Duty Cycle is %3.2lf \r\n", pwm.getDutyCycle());
		}	
		
		//continue without changing anything	
	}
	
    return 0;
}
