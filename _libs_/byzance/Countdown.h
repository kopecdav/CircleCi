#ifndef BYZANCE_COUNTDOWN_H
#define BYZANCE_COUNTDOWN_H

#include "mbed.h"

#include "ByzanceLogger.h"
#include "byzance_debug.h"
#include "macros_bootloader.h"

/** 
* \class Countdown
* \brief Brief TO DO 
*
* Description TO DO 
*/
class Countdown {

  public:

      /** TODO add brief
       *
       * TODO add long description
       *
       * \return Countdown
       */
      Countdown();

      ~Countdown();

      /** TODO add brief
       *
       * TODO add long description
       *
       * \param ms TODO doplnit
       *
       * \return bool TODO doplnit
       */
      Countdown(unsigned long ms);

      /** Expired
       *
       * TODO add long description
       *
       * \return bool TODO doplnit
       */
      bool  expired();

      /** Expireed
       *
       * TODO add long description
       *
       * \param ms TODO doplnit
       */
      void  countdown_ms(unsigned long ms);

      /** Expireed
       *
       * TODO add long description
       *
       * \param seconds TODO doplnit
       */
      void  countdown(unsigned long seconds);

      /** left_ms
       *
       * TODO add long description
       *
       * \return bool TODO doplnit
       */
      unsigned long left_ms();
      
  private:
      Timer t;
      unsigned long interval_end_ms = 0;
};

#endif /* BYZANCE_COUNTDOWN_H */
