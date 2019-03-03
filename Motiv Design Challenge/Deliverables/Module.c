#include "Module.h"
#include "Periodic.h"
#include "Pin.h"
#include "Generic.h"
#include <assert.h>
#include <math.h>

/*
    This module was written with the assumption that Periodic.h will call its ISR every time the clock rollsover
    and that the FreqHz parameter in PeriodicConfig() is as is, i.e: whatever is passed in becomes the tick count
    of the clock.
*/

/* CONSTANTS AND TIMING PARAMETERS */
#define HIGH          1
#define LOW           0
#define TAP_MAX     255
#define TAP_LOW       0
#define T_ID         25 // units in uS
#define T_CI         25
#define T_IL         25
#define T_IH         25

typedef struct {
    u8                     desiredTap;
    u8                     tapSetting;
    u16                    counter;
    module_MAX5389_state_E currState;
} module_MAX5389_data_S;


module_MAX5389_data_S dataChannelA =
{
    .desiredTap  = 128,
    .tapSetting = 128,
    .counter    = 0,
    .currState  = MODULE_MAX5389_INIT
};

module_MAX5389_data_S dataChannelB =
{
    .desiredTap  = 128,
    .tapSetting = 128,
    .counter    = 0,
    .currState  = MODULE_MAX5389_INIT
};


/*    Module State Machine     */
module_MAX5389_state_E module_MAX5849_getChannelADesiredState() {
    module_MAX5389_state_E state = module_MAX5389_getChannelACurrentState();
    module_MAX5389_state_E ret = state;

    switch(state) {
        case MODULE_MAX5389_INIT:
            module_MAX5389_resetChannelACounter();
            PinWrite(PinCSA, HIGH);
            if(dataChannelA.desiredTap == dataChannelA.tapSetting) {
                PeriodicIruptDisable();
                PeriodicStop();
            } else {
                ret = MODULE_MAX5389_LIMBO;
            }
            break;

        case MODULE_MAX5389_LIMBO:
            /*
                This state will go down whatever route it needs to in order to reach the desired R value.
            */
            PinWrite(PinCSA, HIGH);
            if(dataChannelA.desiredTap < dataChannelA.tapSetting) {
                ret = MODULE_MAX5389_DRIVE_UDX_LOW;
            }
            else if(dataChannelA.desiredTap > dataChannelA.tapSetting) {
                PinWrite(PinCSA, HIGH);
                ret = MODULE_MAX5389_DRIVE_UDX_HIGH;
            } else {
                ret = MODULE_MAX5389_INIT;
            }
            break;

        case MODULE_MAX5389_DRIVE_UDX_LOW:
            PinWrite(PinUDA, LOW);
            dataChannelA.counter++;

            /*
                t_ID is 0uS in total but INCA has a setup time for t_CI which is 25 uS.
            */
            if(dataChannelA.counter >= 25) {
                module_MAX5389_resetChannelACounter();
                ret = MODULE_MAX5389_DRIVE_INCX_LOW;
            }

        case MODULE_MAX5389_DRIVE_UDX_HIGH:
            PinWrite(PinUDA, HIGH);
            dataChannelA.counter++;

            /*
                t_DI is 50uS in total but INCA has a setup time for t_CI which is 25 uS.
            */
            if(dataChannelA.counter >= T_ID) {
                module_MAX5389_resetChannelACounter();
                ret = MODULE_MAX5389_DRIVE_INCX_LOW;
            }
            break;

        case MODULE_MAX5389_DRIVE_INCX_LOW:
            PinWrite(PinINCA, LOW);
            dataChannelA.counter++;

            /*
                t_CI is 25 uS
                Also need to hold t_IL for 25uS
                Total = 25 + 25 = 50uS
            */
            if(dataChannelA.counter >= (T_CI + T_IL)) {
                module_MAX5389_resetChannelACounter();
                ret = MODULE_MAX5389_DRIVE_INCX_HIGH;
            }
            break;

        case MODULE_MAX5389_DRIVE_INCX_HIGH:
            PinWrite(PinINCA, HIGH);
            dataChannelA.counter ++;

            /*
                t_IH for INC_ is 25uS. So we need to drive INC_ high for 25uS.
            */
            if(dataChannelA.counter >= T_IH) {
                dataChannelA.tapSetting++;
                module_MAX5389_resetChannelACounter();
                ret = MODULE_MAX5389_LIMBO;
            }
            break;

         default:
            break;
    }
    return ret;
}

module_MAX5389_state_E module_MAX5849_getChannelBDesiredState() {
    module_MAX5389_state_E state = module_MAX5389_getChannelBCurrentState();
    module_MAX5389_state_E ret = state;

    switch(state) {
        case MODULE_MAX5389_INIT:
            module_MAX5389_resetChannelBCounter();
            PinWrite(PinCSB, HIGH);
            if(dataChannelB.desiredTap == dataChannelB.tapSetting) {
                PeriodicIruptDisable();
                PeriodicStop();
            } else {
                ret = MODULE_MAX5389_LIMBO;
            }
            break;

        case MODULE_MAX5389_LIMBO:
            /*
                This state will go down whatever route it needs to in order to reach the desired R value.
            */
            PinWrite(PinCSB, HIGH);
            if(dataChannelB.desiredTap < dataChannelB.tapSetting) {
                ret = MODULE_MAX5389_DRIVE_UDX_LOW;
            }
            else if(dataChannelB.desiredTap > dataChannelB.tapSetting) {
                PinWrite(PinCSB, HIGH);
                ret = MODULE_MAX5389_DRIVE_UDX_HIGH;
            } else {
                ret = MODULE_MAX5389_INIT;
            }
            break;

        case MODULE_MAX5389_DRIVE_UDX_LOW:
            PinWrite(PinUDB, LOW);
            dataChannelB.counter++;

            /*
                t_ID is 0uS in total but INCA has a setup time for t_CI which is 25 uS.
            */
            if(dataChannelB.counter >= 25) {
                module_MAX5389_resetChannelBCounter();
                ret = MODULE_MAX5389_DRIVE_INCX_LOW;
            }

        case MODULE_MAX5389_DRIVE_UDX_HIGH:
            PinWrite(PinUDB, HIGH);
            dataChannelB.counter++;

            /*
                t_DI is 50uS in total but INCA has a setup time for t_CI which is 25 uS.
            */
            if(dataChannelB.counter >= T_ID) {
                module_MAX5389_resetChannelBCounter();
                ret = MODULE_MAX5389_DRIVE_INCX_LOW;
            }
            break;

        case MODULE_MAX5389_DRIVE_INCX_LOW:
            PinWrite(PinINCB, LOW);
            dataChannelB.counter++;

            /*
                t_CI is 25 uS
                Also need to hold t_IL for 25uS
                Total = 25 + 25 = 50uS
            */
            if(dataChannelB.counter >= (T_CI + T_IL)) {
                module_MAX5389_resetChannelACounter();
                ret = MODULE_MAX5389_DRIVE_INCX_HIGH;
            }
            break;

        case MODULE_MAX5389_DRIVE_INCX_HIGH:
            PinWrite(PinINCB, HIGH);
            dataChannelB.counter ++;

            /*
                t_IH for INC_ is 25uS. So we need to drive INC_ high for 25uS.
            */
            if(dataChannelB.counter >= T_IH) {
                dataChannelB.tapSetting++;
                module_MAX5389_resetChannelBCounter();
                ret = MODULE_MAX5389_LIMBO;
            }
            break;

         default:
            break;
    }
    return ret;
}

void module_MAX5389_runChannelAStateMachine ( void ) {
    module_MAX5389_state_E desiredState = module_MAX5849_getChannelADesiredState();
    if(desiredState != dataChannelA.currState) {
        ; // Leave here for future entry actions.
    }
}

void module_MAX5389_runChannelBStateMachine ( void ) {
    module_MAX5389_state_E desiredState = module_MAX5849_getChannelBDesiredState();
    if(desiredState != dataChannelB.currState) {
        ; // Leave here for future entry actions. 
    }
}

/*     Module functions       */

/*
    Resistance in kOhms.s
*/
void module_MAX5389_setChannelAResistance(int desiredResistance) {
    u8 desiredTap = (u8)convert(desiredResistance);
    assert(desiredTap <= TAP_MAX && desiredTap >= TAP_LOW);
    module_MAX5389_setChannelADesiredTap(desiredTap);

    PinModuleInit();

    PeriodicModuleInit();
    PeriodicConfig(PeriodicFreqHzMax, module_MAX5389_runChannelAStateMachine);
    PeriodicIruptEnable();
    PeriodicStart();
}

void module_MAX5389_setChannelBResistance(int desiredResistance) {
    u8 desiredTap = (u8)convert(desiredResistance);
    assert(desiredTap <= TAP_MAX && desiredTap >= TAP_LOW);
    module_MAX5389_setChannelBDesiredTap(desiredTap);

    PinModuleInit();

    PeriodicModuleInit();
    PeriodicConfig(PeriodicFreqHzMax, module_MAX5389_runChannelBStateMachine);
    PeriodicIruptEnable();
    PeriodicStart();
}

/*   Getter and Setter Functions   */
void module_MAX5389_setChannelADesiredTap(int desiredTap) {
    dataChannelA.desiredTap = desiredTap;
}

void module_MAX5389_resetChannelACounter() {
    dataChannelA.counter = 0;
}

module_MAX5389_state_E module_MAX5389_getChannelACurrentState() {
    return dataChannelA.currState;
}

void module_MAX5389_setChannelBDesiredTap(int desiredTap) {
    dataChannelA.desiredTap = desiredTap;
}

void module_MAX5389_resetChannelBCounter() {
    dataChannelA.counter = 0;
}

module_MAX5389_state_E module_MAX5389_getChannelBCurrentState() {
    return dataChannelA.currState;
}

/* Math functions */
double round(double d)
{
	return floor(d + 0.5);
}

int convert(int input) {
	int input_range = 100 - 10;
	int output_range = 255 - 0;

	int output;
	double slope = 1.0 * (255 - 0) / (100 - 10);
	output = 100 + round(slope * (input - 10));
	return output;	
}
