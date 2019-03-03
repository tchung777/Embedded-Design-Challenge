#include <stdio.h>
#include "Module.h"

int main(int argc, char *argv[]) {
	module_MAX5389_setChannelAResistance(50); //Resistance ins kOhms.
    //module_MAX5389_setChannelBResistance(90);
    return 0;
}