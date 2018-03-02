/**
  ******************************************************************************
  * @file    build_date.c
  * @author  Ing. Martin Borýsek
  * @version V1.0.0
  * @date    05. 02. 2016
  * @brief   This file defines macros
  *           BUILD_DAY, BUILD_MONTH, BUILD_YEAR_LEN2, BUILD_YEAR_LEN4
  *           BUILD_HOUR, BUILD_MINUTE, BUILD_SECOND
  ******************************************************************************/

#ifndef __BUILD_DATE_H
#define __BUILD_DATE_H

#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define __BUILD_MONTH__ \
    ( \
        (BUILD_MONTH_IS_JAN) ?  1 : \
        (BUILD_MONTH_IS_FEB) ?  2 : \
        (BUILD_MONTH_IS_MAR) ?  3 : \
        (BUILD_MONTH_IS_APR) ?  4 : \
        (BUILD_MONTH_IS_MAY) ?  5 : \
        (BUILD_MONTH_IS_JUN) ?  6 : \
        (BUILD_MONTH_IS_JUL) ?  7 : \
        (BUILD_MONTH_IS_AUG) ?  8 : \
        (BUILD_MONTH_IS_SEP) ?  9 : \
        (BUILD_MONTH_IS_OCT) ? 10 : \
        (BUILD_MONTH_IS_NOV) ? 11 : \
        (BUILD_MONTH_IS_DEC) ? 12 : \
        /* error default */  99 \
    )

#define __BUILD_YEAR_LEN2__ \
    ( \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )

#define __BUILD_YEAR_LEN4__ \
    ( \
        (__DATE__[ 7] - '0') * 1000 + \
        (__DATE__[ 8] - '0') *  100 + \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )

#define __BUILD_DAY__ \
    ( \
        ((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + \
        (__DATE__[5] - '0') \
    )
    
#define __BUILD_HOUR__ \
    ( \
        (__TIME__[ 0] - '0') * 10 + \
        (__TIME__[ 1] - '0') \
    )
    
#define __BUILD_MINUTE__ \
    ( \
        (__TIME__[ 3] - '0') * 10 + \
        (__TIME__[ 4] - '0') \
    )
    
#define __BUILD_SECOND__ \
    ( \
        (__TIME__[ 6] - '0') * 10 + \
        (__TIME__[ 7] - '0') \
    )

#endif /* __BUILD_DATE_H */
