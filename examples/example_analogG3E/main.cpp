#include "byzance.h"

Serial		pc(SERIAL_TX, SERIAL_RX); // tx, rx

AnalogIn	ain_pb01(X00);
AnalogIn	ain_pb00(X01);
AnalogIn	ain_pa03(X04);
AnalogIn	ain_pa06(X05);

AnalogIn	ain_pc00(Y19);
AnalogIn	ain_pc03(Y22);
AnalogIn	ain_pa04(Y23);
AnalogIn	ain_pc02(Y24);
AnalogIn	ain_pa05(Y25);

void init(){

	pc.baud(115200);

	pc.printf("*********************************\n");
	pc.printf("* TEST_ANALOGG3E\n");
	pc.printf("*********************************\n");
}

void loop(){

	AnalogIn	ain_pb01(X00);
	AnalogIn	ain_pb00(X01);
	AnalogIn	ain_pa03(X04);
	AnalogIn	ain_pa06(X05);

	AnalogIn	ain_pc00(Y19);
	AnalogIn	ain_pc03(Y22);
	AnalogIn	ain_pa04(Y23);
	AnalogIn	ain_pc02(Y24);
	AnalogIn	ain_pa05(Y25);

	pc.printf("*********************************\n");
	pc.printf("ain pb01 - X00 = %3.3f% (V)\n", ain_pb01.read());
	pc.printf("ain pb00 - X01 = %3.3f% (V)\n", ain_pb00.read());
	pc.printf("ain pa03 - X04 = %3.3f% (V)\n", ain_pa03.read());
	pc.printf("ain pa06 - X05 = %3.3f% (V)\n", ain_pa06.read());

	pc.printf("ain pc00 - Y19 = %3.3f% (V)\n", ain_pc00.read());
	pc.printf("ain pc03 - Y22 = %3.3f% (V)\n", ain_pc03.read());
	pc.printf("ain pa04 - Y23 = %3.3f% (V)\n", ain_pa04.read());
	pc.printf("ain pc02 - Y24 = %3.3f% (V)\n", ain_pc02.read());
	pc.printf("ain pa05 - Y25 = %3.3f% (V)\n", ain_pa05.read());

	Thread::wait(1000);
}



