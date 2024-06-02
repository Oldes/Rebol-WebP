//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// SPDX-License-Identifier: MIT
// =============================================================================
// Rebol/WebP extension
// =============================================================================

#include "webp-rebol-extension.h"

RL_LIB *RL; // Link back to reb-lib from embedded extensions

//==== Globals ===============================================================//
extern MyCommandPointer Command[];

u32* arg_words;
u32* type_words;
u32* hint_words;

REBCNT Handle_WebPAnimEncoder;

//============================================================================//

static const char* init_block = WEBP_EXT_INIT_CODE;

int Common_mold(REBHOB *hob, REBSER *ser);

int WebPAnimEncoder_free(void* hndl);
int WebPAnimEncoder_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int WebPAnimEncoder_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);



RXIEXT const char *RX_Init(int opts, RL_LIB *lib) {
	RL = lib;
	REBYTE ver[8];
	RL_VERSION(ver);
/*
	debug_print(
		"RXinit webp-extension; Rebol v%i.%i.%i\n",
		ver[1], ver[2], ver[3]);
*/
	if (MIN_REBOL_VERSION > VERSION(ver[1], ver[2], ver[3])) {
		debug_print(
			"Needs at least Rebol v%i.%i.%i!\n",
			 MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD);
		return 0;
	}
	if (!CHECK_STRUCT_ALIGN) {
		trace("CHECK_STRUCT_ALIGN failed!");
		return 0;
	}
#ifdef USE_TRACES
	const int version = WebPGetEncoderVersion();
	const int sharpyuv_version = SharpYuvGetVersion();
	debug_print("libwebp: %d.%d.%d ",
				 (version >> 16) & 0xff, (version >> 8) & 0xff, version & 0xff);
	debug_print("libsharpyuv: %d.%d.%d\n",
				 (sharpyuv_version >> 24) & 0xff, (sharpyuv_version >> 16) & 0xffff,
				 sharpyuv_version & 0xff);
#endif

	REBHSP spec;
	spec.mold = Common_mold;

	spec.size      = sizeof(void*);
	spec.flags     = HANDLE_REQUIRES_HOB_ON_FREE;
	spec.free      = WebPAnimEncoder_free;
	spec.get_path  = WebPAnimEncoder_get_path;
	spec.set_path  = WebPAnimEncoder_set_path;
	Handle_WebPAnimEncoder = RL_REGISTER_HANDLE_SPEC((REBYTE*)"WebPAnimEncoder", &spec);

	return init_block;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	return Command[cmd](frm, ctx);
}
