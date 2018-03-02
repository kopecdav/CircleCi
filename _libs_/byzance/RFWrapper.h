/*
 * RFWrapper.h
 *
 *  Created on: 21. 2. 2018
 *      Author: martin
 */

#ifndef BYZANCE_RFWRAPPER_H_
#define BYZANCE_RFWRAPPER_H_
#include "mbed_interface.h"
#include "NanostackRfPhyAtmel.h"


class RFWrapper: public NanostackRfPhyAtmel{
  public:
	RFWrapper():NanostackRfPhyAtmel(RF_SPI4_MOSI, RF_SPI4_MISO, RF_SPI4_SCK, RF_SPI4_NSS,RF_RST, RF_SLP_TR, RF_IRQ, RF_I2C2_SDA, RF_I2C2_SCL)
  {
  };
};

#endif /* BYZANCE_RFWRAPPER_H_ */
