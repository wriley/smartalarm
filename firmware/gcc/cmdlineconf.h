#ifndef CMDLINECONF_H
#define CMDLINECONF_H

#include "global.h"

// constants/macros/typdefs

// size of command database
// (maximum number of commands the cmdline system can handle)
#define CMDLINE_MAX_COMMANDS	10

// maximum length (number of characters) of each command string
// (quantity must include one additional byte for a null terminator)
#define CMDLINE_MAX_CMD_LENGTH	15

// allotted buffer size for command entry
// (must be enough chars for typed commands and the arguments that follow)
#define CMDLINE_BUFFERSIZE		80

// number of lines of command history to keep
// (each history buffer is CMDLINE_BUFFERSIZE in size)
// ***** ONLY ONE LINE OF COMMAND HISTORY IS CURRENTLY SUPPORTED
#define CMDLINE_HISTORYSIZE		1

#endif
