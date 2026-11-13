/* Host test: SU_LED_Command sysfs GPIO flow (T40 vendor 1.3.x) against a fixture dir.
 * Build: gcc -DSU_LED_GPIO_DIR='"<dir>/"' ... ; <dir> holds export, unexport, gpio7/{active_low,direction,value}. */
#include <stdio.h>
#include <string.h>
#include "sysutils/su_misc.h"
static int f;
#define CHECK(c) do{if(!(c)){printf("FAIL %d: %s\n",__LINE__,#c);f++;}}while(0)
#define D SU_LED_GPIO_DIR
static void rd(const char*p,char*o){FILE*x=fopen(p,"r");o[0]=0;if(x){size_t n=fread(o,1,63,x);o[n]=0;fclose(x);}}
int main(void){
	char b[64];
	CHECK(SU_LED_Command(7,1)==0); rd(D"export",b); CHECK(!strcmp(b,"7"));
	rd(D"gpio7/active_low",b); CHECK(!strcmp(b,"0")); rd(D"gpio7/direction",b); CHECK(!strcmp(b,"out"));
	rd(D"gpio7/value",b); CHECK(!strcmp(b,"1"));
	CHECK(SU_LED_Command(7,0)==0); rd(D"gpio7/value",b); CHECK(!strcmp(b,"0"));
	CHECK(SU_LED_Command(7,2)==0); rd(D"gpio7/value",b); CHECK(!strcmp(b,"0")); rd(D"unexport",b); CHECK(!strcmp(b,"7"));
	CHECK(SU_LED_Command(8,1)==-1);      /* no gpio8 dir: direction/active_low open fails like the vendor */
	if(!f)printf("OK (0 failures)\n");
	return f;}
