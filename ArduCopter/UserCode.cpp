#include <ch.h>
#include "Copter.h"
#include <AP_Filesystem/AP_Filesystem.h>
#include <AP_CANManager/AP_SLCANIface.h>
#include <AP_SerialManager/AP_SerialManager.h>

#define READ_BUFF_SIZE 12
#define RESP_BUF_SIZE 256

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

size_t cobsEncode(const void *data, size_t length, uint8_t *buffer)
{
	assert(data && buffer);

	uint8_t *encode = buffer; // Encoded byte pointer
	uint8_t *codep = encode++; // Output code pointer
	uint8_t code = 1; // Code value

	for (const uint8_t *byte = (const uint8_t *)data; length--; ++byte)
	{
		if (*byte) // Byte not zero, write it
			*encode++ = *byte, ++code;

		if (!*byte || code == 0xff) // Input is zero or block completed, restart
		{
			*codep = code, code = 1, codep = encode;
			if (!*byte || length)
				++encode;
		}
	}
	*codep = code; // Write final code value

	return (size_t)(encode - buffer);
}


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
	//static uint8_t encodedBuff[RESP_BUF_SIZE+12];
	static uint8_t resp[200];
	int32_t n;
	int32_t totalBytes;
	//uint16_t offset = 0;
    


    // read all the file names from the directory
 
    while (true) 
    {
        chThdSleepMilliseconds(50);
        //uart->write(cData, 24 );
        // wait for the command from CC
		memset(readBuff, 0, sizeof(readBuff) ); 
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
						memcpy(&writeBuff[2], resp, 100);
						uart->write((uint8_t*)writeBuff, sizeof(writeBuff)  );
						break;
					}
					case 0x0002:
					{
						int32_t fd;
						
						fd = AP::FS().open("/APM/LOGS/00000001.BIN", O_RDONLY);
						if(fd < 0)
						{
							
							printf("Failed to open the file\r\n");
							writeBuff[0] = 0xAA;
							writeBuff[1] = 0x55;
							writeBuff[2] = 0xFF;
							writeBuff[3] = 0xFF;
							uart->write((uint8_t*)writeBuff, sizeof(writeBuff)  );
						}
						else
						{
							printf("opened the file sucessfully\r\n");
							totalBytes = 0;
							while(1)
							{
								n = AP::FS().read(fd, writeBuff, RESP_BUF_SIZE );
								//printf("n =%ld\r\n", n );
								totalBytes = n + totalBytes;
								if( n == 0 )
								{
									break;
								}
								
								uart->write((uint8_t*)writeBuff, n  );
								//printf("totalBytes read =%ld\r\n", totalBytes );
								chThdSleepMicroseconds(100);
								
							}
							
							// At the End send the EOF
							chThdSleepMilliseconds(200);
							
							
							memset(writeBuff, 0xFF, sizeof(writeBuff) );
							for(int i=0; i < 5; i++)
							{
								uart->write((uint8_t*)writeBuff,  sizeof(writeBuff) );
							}
							printf("Reading data done =%d\r\n", totalBytes);
						}
						
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
			continue;
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
