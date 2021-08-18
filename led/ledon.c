#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>

#define PLPM_BASE 0xc8100014

#define __IO volatile


 
void * var_addr_satr=0;
unsigned int var_addr_size=0;
unsigned int * gpioaddr;
 
void *gpio_base= 0;

int gpio_init(void)
{
	int fd;
    unsigned int addr_start,addr_offset;
    unsigned int PageSize,PageMask;
 
    fd=open("/dev/mem",O_RDWR);
    if(fd<0)
	{
        return -1;
    }
 
    PageSize = sysconf(_SC_PAGESIZE);
    PageMask = ~(PageSize-1);
 
    //printf("take PageSize=%d\n",PageSize);
 
    addr_start =  PLPM_BASE & PageMask ;
    addr_offset=  PLPM_BASE & ~PageMask;
 
    var_addr_size =  PageSize*2;
 
    var_addr_satr = (void*) mmap(0,\
        var_addr_size,\
        PROT_READ | PROT_WRITE,\
        MAP_SHARED,\
        fd,\
        addr_start);
    if(var_addr_satr == MAP_FAILED)
    {
        return -1;
    }
 
    gpio_base = var_addr_satr;
    gpio_base += addr_offset;
 
    //printf("take var addr = 0x%8x\n",(unsigned int)var_addr_satr);
    //printf("make gpio_base = 0x%8x\n",(unsigned int)gpio_base);
 
    close(fd);
    return 0;
}

int gpio_deinit(void)
{
    int fd;
 
 	fd=open("/dev/mem",O_RDWR);
    if(fd<0)
	{
        return -1;
    }
 
 
    if(munmap(var_addr_satr,var_addr_size) == 0)
    {
        //printf("remove var addr ok\n");
    }
	else
    {
    	//printf("remove var addr erro\n");
    }
 
    close(fd);
    return 0;
}

void red_on(void)
{
	unsigned int temp;
	
	temp=*(gpioaddr+4)|0x00040000;
    *(gpioaddr+4)=temp;
}

void green_on(void)
{
	unsigned int temp;
	
	temp=*(gpioaddr+4)|0x00080000;
    *(gpioaddr+4)=temp;
}

void blue_on(void)
{
	unsigned int temp;
	
	temp=*(gpioaddr+4)|0x00100000;
    *(gpioaddr+4)=temp;
}

void red_off(void)
{
	unsigned int temp;
	
	temp=*(gpioaddr+4)&(~0x00040000);
    *(gpioaddr+4)=temp;
}

void green_off(void)
{
	unsigned int temp;
	
	temp=*(gpioaddr+4)&(~0x00080000);
    *(gpioaddr+4)=temp;
}

void blue_off(void)
{
	unsigned int temp;
	
	temp=*(gpioaddr+4)&(~0x00100000);
    *(gpioaddr+4)=temp;
}

unsigned char key_scan(void)
{
	unsigned char key=0;
	
	key=((*(gpioaddr+5)&0x00000020)>>5);
	return key;
} 


int main(int argc,char * argv[])
{
	if(argc<2) return 1;
	if(argc>4) return 1;
	unsigned int temp;
	
	if(gpio_init()!=0)
	{
		printf("gpio_init failed\n");
		return -1;
	}
 	
 	gpioaddr=(unsigned int *)gpio_base;
    
    temp=*gpioaddr&(~0x01800066);//关闭GPIOAO_4,GPIOAO_5复用 
    *gpioaddr=temp;
    
    temp=*(gpioaddr+4)&(~0x0000001c);
    *(gpioaddr+4)=temp;//使能GPIOAO_2,GPIOAO_3,GPIOAO_4输出,GPIOAO_5输入 
    temp=*(gpioaddr+4)|0x00000020;
    *(gpioaddr+4)=temp;
    
    if(argc==2)
    {
	    if(strcmp((const char*)argv[1],"red")==0)
	    red_on();
	    
	    if(strcmp((const char*)argv[1],"green")==0)
	    green_on();
	    
	    if(strcmp((const char*)argv[1],"blue")==0)
	    blue_on();
	}
	
	if(argc==3)
    {
	    if(strcmp((const char*)argv[1],"red")==0||
	    	strcmp((const char*)argv[2],"red")==0)
	    red_on();
	    
	    if(strcmp((const char*)argv[1],"green")==0||
	    	strcmp((const char*)argv[2],"green")==0)
	    green_on();
	    
	    if(strcmp((const char*)argv[1],"blue")==0||
	    	strcmp((const char*)argv[2],"blue")==0)
	    blue_on();
	}
    
    if(argc==4)
    {
	    if(strcmp((const char*)argv[1],"red")==0||
	    	strcmp((const char*)argv[2],"red")==0||
	    	strcmp((const char*)argv[3],"red")==0)
	    red_on();
	    
	    if(strcmp((const char*)argv[1],"green")==0||
	    	strcmp((const char*)argv[2],"green")==0||
	    	strcmp((const char*)argv[3],"green")==0)
	    green_on();
	    
	    if(strcmp((const char*)argv[1],"blue")==0||
	    	strcmp((const char*)argv[2],"blue")==0||
	    	strcmp((const char*)argv[3],"blue")==0)
	    blue_on();
	}	
    /*
    int i=0,val=0;
	unsigned char led[3],led_last[3];
	
	srand(time(NULL));
	for(int t=0;t<100;t++)
	{
		val=rand();
		printf("%d\n",val);
		srand(val);
		
		
		led[0]=val&0x00000001;
		led[1]=(val&0x00000002)>>1;
		led[2]=(val&0x00000004)>>2;
		
		if((led[0]+led[1]+led[2])==0)
		continue;
		
		if(led[0]==led_last[0]&&led[1]==led_last[1]&&led[2]==led_last[2])
		continue;
		
		if(led[0]==1) red_on();
		else red_off();
		
		if(led[1]==1) green_on();
		else green_off();
		
		if(led[2]==1) blue_on();
		else blue_off();
		
		printf("%d\t%d\t%d\n",led[0],led[1],led[2]);
		
		led_last[0]=led[0];
		led_last[1]=led[1];
		led_last[2]=led[2];
		
		usleep(500*1000);
	}
	*/	

	

    gpio_deinit( );
 
    return 0;
}
