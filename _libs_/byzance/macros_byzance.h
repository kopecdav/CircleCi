#ifndef MACROS_BYZANCE_H
#define MACROS_BYZANCE_H

#include "ByzanceSerializer.h"
#include "ByzanceParser.h"
#include "ByzanceMessageInputRegistration.h"
#include "ByzanceAnalogInputRegistration.h"
#include "ByzanceDigitalInputRegistration.h"
#include "ByzanceTypes.h"
#include "macros_bootloader.h"

// DIGITAL IN

#define DIGITAL_INPUT(NAME, METHOD)\
void digital_input_ ## NAME (bool value) {\
    METHOD\
}\
ByzanceDigitalInputRegistration digital_input_ ## NAME ## _registration( #NAME, digital_input_ ## NAME );\

// DIGITAL OUT

#define DIGITAL_OUTPUT(NAME)\
int NAME (bool arg) {\
    return ByzanceCore::digital_output_set_value( #NAME, arg );\
}

// ANALOG IN

#define ANALOG_INPUT(NAME, METHOD)\
void analog_input_ ## NAME (float value) {\
    METHOD\
}\
ByzanceAnalogInputRegistration analog_input_ ## NAME ## _registration( #NAME, analog_input_ ## NAME );\

// ANALOG OUT

#define ANALOG_OUTPUT(NAME)\
int NAME (float arg) {\
    return ByzanceCore::analog_output_set_value( #NAME, arg );\
}

// MESSAGE IN

#define MESSAGE_INPUT_1A(NAME, ARG1, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    METHOD\
    ARG1 ## Free(arg1);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_2A(NAME, ARG1, ARG2, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    ARG2 arg2 = byzanceParser->get ## ARG2(2);\
    METHOD\
    ARG1 ## Free(arg1);\
    ARG2 ## Free(arg2);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_3A(NAME, ARG1, ARG2, ARG3, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    ARG2 arg2 = byzanceParser->get ## ARG2(2);\
    ARG3 arg3 = byzanceParser->get ## ARG3(3);\
    METHOD\
    ARG1 ## Free(arg1);\
    ARG2 ## Free(arg2);\
    ARG3 ## Free(arg3);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_4A(NAME, ARG1, ARG2, ARG3, ARG4, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    ARG2 arg2 = byzanceParser->get ## ARG2(2);\
    ARG3 arg3 = byzanceParser->get ## ARG3(3);\
    ARG4 arg4 = byzanceParser->get ## ARG4(4);\
    METHOD\
    ARG1 ## Free(arg1);\
    ARG2 ## Free(arg2);\
    ARG3 ## Free(arg3);\
    ARG4 ## Free(arg4);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_5A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    ARG2 arg2 = byzanceParser->get ## ARG2(2);\
    ARG3 arg3 = byzanceParser->get ## ARG3(3);\
    ARG4 arg4 = byzanceParser->get ## ARG4(4);\
    ARG5 arg5 = byzanceParser->get ## ARG5(5);\
    METHOD\
    ARG1 ## Free(arg1);\
    ARG2 ## Free(arg2);\
    ARG3 ## Free(arg3);\
    ARG4 ## Free(arg4);\
    ARG5 ## Free(arg5);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_6A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    ARG2 arg2 = byzanceParser->get ## ARG2(2);\
    ARG3 arg3 = byzanceParser->get ## ARG3(3);\
    ARG4 arg4 = byzanceParser->get ## ARG4(4);\
    ARG5 arg5 = byzanceParser->get ## ARG5(5);\
    ARG6 arg6 = byzanceParser->get ## ARG6(6);\
    METHOD\
    ARG1 ## Free(arg1);\
    ARG2 ## Free(arg2);\
    ARG3 ## Free(arg3);\
    ARG4 ## Free(arg4);\
    ARG5 ## Free(arg5);\
    ARG6 ## Free(arg6);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_7A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    ARG2 arg2 = byzanceParser->get ## ARG2(2);\
    ARG3 arg3 = byzanceParser->get ## ARG3(3);\
    ARG4 arg4 = byzanceParser->get ## ARG4(4);\
    ARG5 arg5 = byzanceParser->get ## ARG5(5);\
    ARG6 arg6 = byzanceParser->get ## ARG6(6);\
    ARG7 arg7 = byzanceParser->get ## ARG7(7);\
    METHOD\
    ARG1 ## Free(arg1);\
    ARG2 ## Free(arg2);\
    ARG3 ## Free(arg3);\
    ARG4 ## Free(arg4);\
    ARG5 ## Free(arg5);\
    ARG6 ## Free(arg6);\
    ARG7 ## Free(arg7);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_8A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, METHOD)\
void message_input_ ## NAME (ByzanceParser* byzanceParser) {\
    ARG1 arg1 = byzanceParser->get ## ARG1(1);\
    ARG2 arg2 = byzanceParser->get ## ARG2(2);\
    ARG3 arg3 = byzanceParser->get ## ARG3(3);\
    ARG4 arg4 = byzanceParser->get ## ARG4(4);\
    ARG5 arg5 = byzanceParser->get ## ARG5(5);\
    ARG6 arg6 = byzanceParser->get ## ARG6(6);\
    ARG7 arg7 = byzanceParser->get ## ARG7(7);\
    ARG8 arg8 = byzanceParser->get ## ARG8(8);\
    METHOD\
    ARG1 ## Free(arg1);\
    ARG2 ## Free(arg2);\
    ARG3 ## Free(arg3);\
    ARG4 ## Free(arg4);\
    ARG5 ## Free(arg5);\
    ARG6 ## Free(arg6);\
    ARG7 ## Free(arg7);\
    ARG8 ## Free(arg8);\
}\
ByzanceMessageInputRegistration message_input_ ## NAME ## _registration( #NAME, message_input_ ## NAME );

#define MESSAGE_INPUT_ERROR(...)    _Pragma ("MESSAGE_INPUT has 3-10 parameters (format is MESSAGE_INPUT(NAME, ARG1_TYPE-ARG8_TYPE, METHOD))")
#define MESSAGE_INPUT_ERROR_0A(...)    MESSAGE_INPUT_ERROR()
#define MESSAGE_INPUT_ERROR_00A(...)   MESSAGE_INPUT_ERROR()

#define GET_MESSAGE_INPUT_MACRO(_00,_0,_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define MESSAGE_INPUT(...) GET_MESSAGE_INPUT_MACRO(__VA_ARGS__, MESSAGE_INPUT_8A, MESSAGE_INPUT_7A, MESSAGE_INPUT_6A, MESSAGE_INPUT_5A, MESSAGE_INPUT_4A, MESSAGE_INPUT_3A, MESSAGE_INPUT_2A, MESSAGE_INPUT_1A, MESSAGE_INPUT_ERROR_0A, MESSAGE_INPUT_ERROR_00A)(__VA_ARGS__)

// MESSAGE OUT

#define MESSAGE_OUTPUT_1A(NAME, ARG1)\
int NAME(ARG1 ## Const arg1) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME;\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	return rc;\
}

#define MESSAGE_OUTPUT_2A(NAME, ARG1, ARG2)\
int NAME(ARG1 ## Const arg1, ARG2 ## Const arg2) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME;\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = byzance_serializer_ ## NAME.add ## ARG2(arg2, 2);\
	if(rc) return -2;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	return rc;\
}

#define MESSAGE_OUTPUT_3A(NAME, ARG1, ARG2, ARG3)\
int NAME(ARG1 ## Const arg1, ARG2 ## Const arg2, ARG3 ## Const arg3) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME;\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = byzance_serializer_ ## NAME.add ## ARG2(arg2, 2);\
	if(rc) return -2;\
	rc = byzance_serializer_ ## NAME.add ## ARG3(arg3, 3);\
	if(rc) return -3;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	return rc;\
}

#define MESSAGE_OUTPUT_4A(NAME, ARG1, ARG2, ARG3, ARG4)\
int NAME(ARG1 ## Const arg1, ARG2 ## Const arg2, ARG3 ## Const arg3, ARG4 ## Const arg4) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME;\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = byzance_serializer_ ## NAME.add ## ARG2(arg2, 2);\
	if(rc) return -2;\
	rc = byzance_serializer_ ## NAME.add ## ARG3(arg3, 3);\
	if(rc) return -3;\
	rc = byzance_serializer_ ## NAME.add ## ARG4(arg4, 4);\
	if(rc) return -4;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	return rc;\
}

#define MESSAGE_OUTPUT_5A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5)\
int NAME(ARG1 ## Const arg1, ARG2 ## Const arg2, ARG3 ## Const arg3, ARG4 ## Const arg4, ARG5 ## Const arg5) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME;\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = byzance_serializer_ ## NAME.add ## ARG2(arg2, 2);\
	if(rc) return -2;\
	rc = byzance_serializer_ ## NAME.add ## ARG3(arg3, 3);\
	if(rc) return -3;\
	rc = byzance_serializer_ ## NAME.add ## ARG4(arg4, 4);\
	if(rc) return -4;\
	rc = byzance_serializer_ ## NAME.add ## ARG5(arg5, 5);\
	if(rc) return -5;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	return rc;\
}

#define MESSAGE_OUTPUT_6A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)\
int NAME(ARG1 ## Const arg1, ARG2 ## Const arg2, ARG3 ## Const arg3, ARG4 ## Const arg4, ARG5 ## Const arg5, ARG6 ## Const arg6) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = byzance_serializer_ ## NAME.add ## ARG2(arg2, 2);\
	if(rc) return -2;\
	rc = byzance_serializer_ ## NAME.add ## ARG3(arg3, 3);\
	if(rc) return -3;\
	rc = byzance_serializer_ ## NAME.add ## ARG4(arg4, 4);\
	if(rc) return -4;\
	rc = byzance_serializer_ ## NAME.add ## ARG5(arg5, 5);\
	if(rc) return -5;\
	rc = byzance_serializer_ ## NAME.add ## ARG6(arg6, 6);\
	if(rc) return -6;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	return rc;\
}

#define MESSAGE_OUTPUT_7A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)\
int NAME(ARG1 ## Const arg1, ARG2 ## Const arg2, ARG3 ## Const arg3, ARG4 ## Const arg4, ARG5 ## Const arg5, ARG6 ## Const arg6, ARG7 ## Const arg7) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME;\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = byzance_serializer_ ## NAME.add ## ARG2(arg2, 2);\
	if(rc) return -2;\
	rc = byzance_serializer_ ## NAME.add ## ARG3(arg3, 3);\
	if(rc) return -3;\
	rc = byzance_serializer_ ## NAME.add ## ARG4(arg4, 4);\
	if(rc) return -4;\
	rc = byzance_serializer_ ## NAME.add ## ARG5(arg5, 5);\
	if(rc) return -5;\
	rc = byzance_serializer_ ## NAME.add ## ARG6(arg6, 6);\
	if(rc) return -6;\
	rc = byzance_serializer_ ## NAME.add ## ARG7(arg7, 7);\
	if(rc) return -7;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	delete byzance_serializer_ ## NAME;\
	return rc;\
}

#define MESSAGE_OUTPUT_8A(NAME, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)\
int NAME(ARG1 ## Const arg1, ARG2 ## Const arg2, ARG3 ## Const arg3, ARG4 ## Const arg4, ARG5 ## Const arg5, ARG6 ## Const arg6, ARG7 ## Const arg7, ARG8 ## Const arg8) {\
	int rc = 0;\
	ByzanceSerializer byzance_serializer_ ## NAME;\
	rc = byzance_serializer_ ## NAME.add ## ARG1(arg1, 1);\
	if(rc) return -1;\
	rc = byzance_serializer_ ## NAME.add ## ARG2(arg2, 2);\
	if(rc) return -2;\
	rc = byzance_serializer_ ## NAME.add ## ARG3(arg3, 3);\
	if(rc) return -3;\
	rc = byzance_serializer_ ## NAME.add ## ARG4(arg4, 4);\
	if(rc) return -4;\
	rc = byzance_serializer_ ## NAME.add ## ARG5(arg5, 5);\
	if(rc) return -5;\
	rc = byzance_serializer_ ## NAME.add ## ARG6(arg6, 6);\
	if(rc) return -6;\
	rc = byzance_serializer_ ## NAME.add ## ARG7(arg7, 7);\
	if(rc) return -7;\
	rc = byzance_serializer_ ## NAME.add ## ARG8(arg8, 8);\
	if(rc) return -8;\
	rc = ByzanceCore::message_output_set_value( #NAME, &byzance_serializer_ ## NAME);\
	delete byzance_serializer_ ## NAME;\
	return rc;\
}

#define MESSAGE_OUTPUT_ERROR(...)   _Pragma ("MESSAGE_OUTPUT has 2-9 parameters (format is MESSAGE_OUTPUT(NAME, ARG1_TYPE-ARG8_TYPE))")
#define MESSAGE_OUTPUT_ERROR_00A(...)   MESSAGE_OUTPUT_ERROR()

#define GET_MESSAGE_OUTPUT_MACRO(_00,_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define MESSAGE_OUTPUT(...) GET_MESSAGE_OUTPUT_MACRO(__VA_ARGS__, MESSAGE_OUTPUT_8A, MESSAGE_OUTPUT_7A, MESSAGE_OUTPUT_6A, MESSAGE_OUTPUT_5A, MESSAGE_OUTPUT_4A, MESSAGE_OUTPUT_3A, MESSAGE_OUTPUT_2A, MESSAGE_OUTPUT_1A, MESSAGE_OUTPUT_ERROR_00A)(__VA_ARGS__)

#endif /* MACROS_BYZANCE_H */
