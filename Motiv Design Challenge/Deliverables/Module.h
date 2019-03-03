#ifndef _MODULE_H
#define _MODULE_H

/* Available States */
typedef enum {
    MODULE_MAX5389_INIT,
    MODULE_MAX5389_DRIVE_UDX_LOW,
    MODULE_MAX5389_DRIVE_UDX_HIGH,
    MODULE_MAX5389_DRIVE_INCX_LOW,
    MODULE_MAX5389_DRIVE_INCX_HIGH,
    MODULE_MAX5389_LIMBO
} module_MAX5389_state_E;

/* Channel A */
module_MAX5389_state_E module_MAX5849_getChannelADesiredState();
void module_MAX5389_runChannelAStateMachine ( void );
void module_MAX5389_setChannelAResistance(int desiredResistance);
void module_MAX5389_setChannelADesiredTap(int desiredTap);
void module_MAX5389_resetChannelACounter();
module_MAX5389_state_E module_MAX5389_getChannelACurrentState();

 /* Channel B */
module_MAX5389_state_E module_MAX5849_getChannelBDesiredState();
void module_MAX5389_setChannelBResistance(int desiredResistance);
void module_MAX5389_runChannelBStateMachine ( void );
void module_MAX5389_setChannelBDesiredTap(int desiredTap);
void module_MAX5389_resetChannelBCounter();
module_MAX5389_state_E module_MAX5389_getChannelBCurrentState();

/* Math functions */
int convert( int input );
double round( double d );

#endif /* endif MODULE_H */