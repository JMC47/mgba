#ifndef ARM_H
#define ARM_H

#include <stdint.h>

enum {
	ARM_SP = 13,
	ARM_LR = 14,
	ARM_PC = 15
};

enum ExecutionMode {
	MODE_ARM = 0,
	MODE_THUMB = 1
};

enum PrivilegeMode {
	MODE_USER = 0x10,
	MODE_FIQ = 0x11,
	MODE_IRQ = 0x12,
	MODE_SUPERVISOR = 0x13,
	MODE_ABORT = 0x17,
	MODE_UNDEFINED = 0x1B,
	MODE_SYSTEM = 0x1F
};

enum WordSize {
	WORD_SIZE_ARM = 4,
	WORD_SIZE_THUMB = 2
};

enum ExecutionVector {
	BASE_RESET = 0x00000000,
	BASE_UNDEF = 0x00000004,
	BASE_SWI = 0x00000008,
	BASE_PABT = 0x0000000C,
	BASE_DABT = 0x00000010,
	BASE_IRQ = 0x00000018,
	BASE_FIQ = 0x0000001C
};

enum RegisterBank {
	BANK_NONE = 0,
	BANK_FIQ = 1,
	BANK_IRQ = 2,
	BANK_SUPERVISOR = 3,
	BANK_ABORT = 4,
	BANK_UNDEFINED = 5
};

struct ARMCore;

union PSR {
	struct {
		enum PrivilegeMode priv : 5;
		enum ExecutionMode t : 1;
		unsigned f : 1;
		unsigned i : 1;
		unsigned : 20;
		unsigned v : 1;
		unsigned c : 1;
		unsigned z : 1;
		unsigned n : 1;
	};

	int32_t packed;
};

struct ARMMemory {
	int32_t (*load32)(struct ARMMemory*, uint32_t address, int* cycleCounter);
	int16_t (*load16)(struct ARMMemory*, uint32_t address, int* cycleCounter);
	uint16_t (*loadU16)(struct ARMMemory*, uint32_t address, int* cycleCounter);
	int8_t (*load8)(struct ARMMemory*, uint32_t address, int* cycleCounter);
	uint8_t (*loadU8)(struct ARMMemory*, uint32_t address, int* cycleCounter);

	void (*store32)(struct ARMMemory*, uint32_t address, int32_t value, int* cycleCounter);
	void (*store16)(struct ARMMemory*, uint32_t address, int16_t value, int* cycleCounter);
	void (*store8)(struct ARMMemory*, uint32_t address, int8_t value, int* cycleCounter);

	uint32_t* activeRegion;
	uint32_t activeMask;
	uint32_t activePrefetchCycles32;
	uint32_t activePrefetchCycles16;
	void (*setActiveRegion)(struct ARMMemory*, uint32_t address);
	int (*waitMultiple)(struct ARMMemory*, uint32_t startAddress, int count);
};

struct ARMBoard {
	struct ARMCore* cpu;
	void (*reset)(struct ARMBoard* board);
	void (*processEvents)(struct ARMBoard* board);
	void (*swi16)(struct ARMBoard* board, int immediate);
	void (*swi32)(struct ARMBoard* board, int immediate);

	void (*hitStub)(struct ARMBoard* board, uint32_t opcode);
};

struct ARMCore {
	int32_t gprs[16];
	union PSR cpsr;
	union PSR spsr;

	int32_t cycles;
	int32_t nextEvent;

	int32_t bankedRegisters[6][7];
	int32_t bankedSPSRs[6];

	int32_t shifterOperand;
	int32_t shifterCarryOut;

	enum ExecutionMode executionMode;
	enum PrivilegeMode privilegeMode;

	struct ARMMemory* memory;
	struct ARMBoard* board;

	int64_t absoluteCycles;
	int32_t lastCycles;
};

void ARMInit(struct ARMCore* cpu);
void ARMAssociateMemory(struct ARMCore* cpu, struct ARMMemory* memory);
void ARMAssociateBoard(struct ARMCore* cpu, struct ARMBoard* board);

void ARMReset(struct ARMCore* cpu);
void ARMSetPrivilegeMode(struct ARMCore*, enum PrivilegeMode);
void ARMRaiseIRQ(struct ARMCore*);
void ARMRaiseSWI(struct ARMCore*);

void ARMRun(struct ARMCore* cpu);

#endif
