#ifndef PORT_RANGES_H
#define PORT_RANGES_H

#include "./types.h"

const Port_Range WELL_KNOWN = {0, 1023};
const Port_Range REGISTERED = {1024, 49151};
const Port_Range PRIVATE = {49151, 65535};

#endif
