#include <ch.h>
#include "Copter.h"
#include <AP_Filesystem/AP_Filesystem.h>
#include <AP_CANManager/AP_SLCANIface.h>
#include <AP_SerialManager/AP_SerialManager.h>



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
    static uint8_t *cData = (uint8_t*)"HELLO MR ZAFAR RAVVIT\r\n";
    auto *uart = hal.serial(6);
    auto *d = AP::FS().opendir("/APM/LOGS");
    

    if (d == nullptr) 
    {
        printf("Failed to open the directory\n");
    }
    else
    {
        printf("Opened the directory sucessfully\r\n");
    }

    // read all the file names from the directory
    while ((de = AP::FS().readdir(d))) 
    {
        //printf("file name =%s\r\n", de->d_name );
        uart->write((uint8_t*)de->d_name, strlen(de->d_name)  );
    }
 
    while (true) 
    {
        chThdSleepMilliseconds(1000);
        uart->write(cData, 24 );
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
