#ifndef MACROS_BOOTLOADER_H
#define MACROS_BOOTLOADER_H

#if !defined(__BUILD_ID__)
    #define __BUILD_ID__ UNKNOWN_BUILD_ID
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define BYZANCE_UNUSED_ARG(x) (void)(x)

#define __TOMAS_ZARUBA__ error("I'm killing myself\n");

#endif /* MACROS_BOOTLOADER */
