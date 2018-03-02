#ifndef ENUM_BINSTATE_H
#define ENUM_BINSTATE_H

typedef enum : unsigned char {
	BINSTRUCT_STATE_INVALID,	// upload or write in progress
	BINSTRUCT_STATE_VALID,		// binary uploaded or written + verified
	BINSTRUCT_STATE_ERASING,	// erase in progress
	BINSTRUCT_STATE_ERASED,		// sucesfully erased
	_BINSTRUCT_STATE_LAST = BINSTRUCT_STATE_ERASED // support field for automatic detection of last item
} bin_state_t;

#endif /* ENUM_BINSTATE_H */
