#ifndef ENUM_WORKERSTATE_H
#define ENUM_WORKERSTATE_H

typedef enum : unsigned char {
	WORKER_STATE_NONE				= 0x00,
	WORKER_STATE_COMMANDS_ERASE		= 0x01,
	WORKER_STATE_COMMANDS_UPLOAD	= 0x02
} WorkerState_t;

#endif /* ENUM_WORKERSTATE_H */
