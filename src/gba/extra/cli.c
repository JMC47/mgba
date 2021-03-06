/* Copyright (c) 2013-2015 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "cli.h"

#include "arm/debugger/cli-debugger.h"
#include "core/serialize.h"
#include "gba/io.h"
#include "gba/serialize.h"

static void _GBACLIDebuggerInit(struct CLIDebuggerSystem*);
static bool _GBACLIDebuggerCustom(struct CLIDebuggerSystem*);
static uint32_t _GBACLIDebuggerLookupIdentifier(struct CLIDebuggerSystem*, const char* name, struct CLIDebugVector* dv);

static void _frame(struct CLIDebugger*, struct CLIDebugVector*);
static void _load(struct CLIDebugger*, struct CLIDebugVector*);
static void _save(struct CLIDebugger*, struct CLIDebugVector*);

struct CLIDebuggerCommandSummary _GBACLIDebuggerCommands[] = {
	{ "frame", _frame, 0, "Frame advance" },
	{ "load", _load, CLIDVParse, "Load a savestate" },
	{ "save", _save, CLIDVParse, "Save a savestate" },
	{ 0, 0, 0, 0 }
};

struct GBACLIDebugger* GBACLIDebuggerCreate(struct mCore* core) {
	struct GBACLIDebugger* debugger = malloc(sizeof(struct GBACLIDebugger));
	ARMCLIDebuggerCreate(&debugger->d);
	debugger->d.init = _GBACLIDebuggerInit;
	debugger->d.deinit = NULL;
	debugger->d.custom = _GBACLIDebuggerCustom;
	debugger->d.lookupIdentifier = _GBACLIDebuggerLookupIdentifier;

	debugger->d.name = "Game Boy Advance";
	debugger->d.commands = _GBACLIDebuggerCommands;

	debugger->core = core;

	return debugger;
}

static void _GBACLIDebuggerInit(struct CLIDebuggerSystem* debugger) {
	struct GBACLIDebugger* gbaDebugger = (struct GBACLIDebugger*) debugger;

	gbaDebugger->frameAdvance = false;
}

static bool _GBACLIDebuggerCustom(struct CLIDebuggerSystem* debugger) {
	struct GBACLIDebugger* gbaDebugger = (struct GBACLIDebugger*) debugger;

	if (gbaDebugger->frameAdvance) {
		if (!gbaDebugger->inVblank && GBARegisterDISPSTATIsInVblank(((struct GBA*) gbaDebugger->core->board)->memory.io[REG_DISPSTAT >> 1])) {
			mDebuggerEnter(&gbaDebugger->d.p->d, DEBUGGER_ENTER_MANUAL, 0);
			gbaDebugger->frameAdvance = false;
			return false;
		}
		gbaDebugger->inVblank = GBARegisterDISPSTATGetInVblank(((struct GBA*) gbaDebugger->core->board)->memory.io[REG_DISPSTAT >> 1]);
		return true;
	}
	return false;
}

static uint32_t _GBACLIDebuggerLookupIdentifier(struct CLIDebuggerSystem* debugger, const char* name, struct CLIDebugVector* dv) {
	UNUSED(debugger);
	int i;
	for (i = 0; i < REG_MAX; i += 2) {
		const char* reg = GBAIORegisterNames[i >> 1];
		if (reg && strcasecmp(reg, name) == 0) {
			return BASE_IO | i;
		}
	}
	dv->type = CLIDV_ERROR_TYPE;
	return 0;
}

static void _frame(struct CLIDebugger* debugger, struct CLIDebugVector* dv) {
	UNUSED(dv);
	debugger->d.state = DEBUGGER_CUSTOM;

	struct GBACLIDebugger* gbaDebugger = (struct GBACLIDebugger*) debugger->system;
	gbaDebugger->frameAdvance = true;
	gbaDebugger->inVblank = GBARegisterDISPSTATGetInVblank(((struct GBA*) gbaDebugger->core->board)->memory.io[REG_DISPSTAT >> 1]);
}

static void _load(struct CLIDebugger* debugger, struct CLIDebugVector* dv) {
	struct CLIDebuggerBackend* be = debugger->backend;
	if (!dv || dv->type != CLIDV_INT_TYPE) {
		be->printf(be, "%s\n", ERROR_MISSING_ARGS);
		return;
	}

	int state = dv->intValue;
	if (state < 1 || state > 9) {
		be->printf(be, "State %u out of range", state);
	}

	struct GBACLIDebugger* gbaDebugger = (struct GBACLIDebugger*) debugger->system;

	mCoreLoadState(gbaDebugger->core, dv->intValue, SAVESTATE_SCREENSHOT);
}

// TODO: Put back rewind

static void _save(struct CLIDebugger* debugger, struct CLIDebugVector* dv) {
	struct CLIDebuggerBackend* be = debugger->backend;
	if (!dv || dv->type != CLIDV_INT_TYPE) {
		be->printf(be, "%s\n", ERROR_MISSING_ARGS);
		return;
	}

	int state = dv->intValue;
	if (state < 1 || state > 9) {
		be->printf(be, "State %u out of range", state);
	}

	struct GBACLIDebugger* gbaDebugger = (struct GBACLIDebugger*) debugger->system;

	mCoreSaveState(gbaDebugger->core, dv->intValue, SAVESTATE_SCREENSHOT);
}
