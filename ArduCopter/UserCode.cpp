#include <ch.h>
#include "Copter.h"
#include <AP_Filesystem/AP_Filesystem.h>
#include <AP_CANManager/AP_SLCANIface.h>
#include <AP_SerialManager/AP_SerialManager.h>

#define READ_BUFF_SIZE 12
#define RESP_BUF_SIZE 512

AP_HAL::UARTDriver *ptr;

static THD_WORKING_AREA(sdCardReaderThread, 512 );
static void cardReaderThread(void *arg);

//#include "AP_Filesystem.h"
extern "C" {
void can_printf(const char *fmt, ...);
}

#ifdef USERHOOK_INIT
void Copter::userhook_init()
{
    // put your initialisation code here
    // this will be called once at start-up
    
    thread_t *tp = chThdCreateStatic(sdCardReaderThread,
                                   sizeof(sdCardReaderThread),
                                   NORMALPRIO,          /* Initial priority.    */
                                   cardReaderThread,    /* Thread function.     */
                                   NULL);
    tp = tp;
    
}
#endif

#ifdef USERHOOK_FASTLOOP
void Copter::userhook_FastLoop()
{
    // put your 100Hz code here
     
}
#endif

#ifdef USERHOOK_50HZLOOP
void Copter::userhook_50Hz()
{
    // put your 50Hz code here
}
#endif

#ifdef USERHOOK_MEDIUMLOOP
void Copter::userhook_MediumLoop()
{
    // put your 10Hz code here
}
#endif

#ifdef USERHOOK_SLOWLOOP
void Copter::userhook_SlowLoop()
{
    // put your 3.3Hz code here
}
#endif

#ifdef USERHOOK_SUPERSLOWLOOP
void Copter::userhook_SuperSlowLoop()
{
    // put your 1Hz code here
}
#endif


static THD_FUNCTION(cardReaderThread, arg) 
{
    //static uint64_t nSize = 0;
    struct dirent *de;
    //static uint8_t *cData = (uint8_t*)"HELLO MR BUNNY RABBIT\r\n";
    auto *uart = hal.serial(6);
	static uint8_t readBuff[READ_BUFF_SIZE];
	int nRet = -1;
	uint16_t command = 0;
	static uint8_t writeBuff[RESP_BUF_SIZE];
	static uint8_t resp[200];
	//uint16_t offset = 0;
    


    // read all the file names from the directory
 
    while (true) 
    {
        chThdSleepMilliseconds(100);
        //uart->write(cData, 24 );
        // wait for the command from CC
		nRet = uart->read(readBuff, READ_BUFF_SIZE);
		
        if( nRet > 0 )
		{
			// command received
			
			// check for the header
			if( (readBuff[0] == 0xAA) && (readBuff[1] == 0xCC) )
			{
				//command = *(uint16_t*)&readBuff[2]);
				command = readBuff[2] | readBuff[3] << 8;
				printf("Command received =%X\r\n", command );
				memset(writeBuff, 0x00, sizeof(writeBuff) );
				memset(resp, 0, 200);
				switch( command )
				{
					case 0x0001:
					{
						auto *d = AP::FS().opendir("/APM/LOGS");
						writeBuff[0] = 0xAA;
						writeBuff[1] = 0x55;
						
						if (d == nullptr) 
						{
							printf("Failed to open the directory\n");
						}
						else
						{
							printf("Opened the directory sucessfully\r\n");
						}
						while ((de = AP::FS().readdir(d))) 
						{
							//printf("file name =%s\r\n", de->d_name );
							//strcpy((char*)&writeBuff[offset], (char*)de->d_name );
							strcat((char*)resp, (char*)de->d_name );
							strcat((char*)resp, " " );
							//uart->write((uint8_t*)de->d_name, strlen(de->d_name)  );
						}
						AP::FS().closedir(d);
						printf("writeBuff =%s\r\n", resp );
						memcpy(&writeBuff[2], resp, 200);
						uart->write((uint8_t*)writeBuff, sizeof(writeBuff)  );
						break;
					}
					default:
						printf("Unknown command sent\r\n");
					break;
				}
					
			}
			
		}
		else
		{
			//printf("command not received\r\n");
		}


    }
}

#ifdef USERHOOK_AUXSWITCH
void Copter::userhook_auxSwitch1(const RC_Channel::AuxSwitchPos ch_flag)
{
    // put your aux switch #1 handler here (CHx_OPT = 47)
}

void Copter::userhook_auxSwitch2(const RC_Channel::AuxSwitchPos ch_flag)
{
    // put your aux switch #2 handler here (CHx_OPT = 48)
}

void Copter::userhook_auxSwitch3(const RC_Channel::AuxSwitchPos ch_flag)
{
    // put your aux switch #3 handler here (CHx_OPT = 49)
}
#endif
