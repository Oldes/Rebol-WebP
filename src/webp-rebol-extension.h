//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/WebP extension
// SPDX-License-Identifier: MIT
// =============================================================================
// NOTE: auto-generated file, do not modify!
//
#include "rebol-extension.h"
#include "webp/encode.h"
#include "webp/decode.h"
#include "webp/mux.h"

#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 14
#define MIN_REBOL_UPD 1
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

extern u32* arg_words;
extern u32* type_words;
extern u32* hint_words;

extern REBCNT Handle_WebPAnimEncoder;

// The encoder is allocated from the WebP library
// so it must be wrapped inside another struct!
typedef struct WebPAnimEncoderWrapper {
	WebPAnimEncoder* encoder;
} WebPAnimEncoderWrapper;

enum ext_commands {
	CMD_WEBP_WEBP_INIT,
	CMD_WEBP_ENCODE,
	CMD_WEBP_DECODE,
	CMD_WEBP_CONFIG,
	CMD_WEBP_ANIM_ENCODER,
	CMD_WEBP_ENCODE_FRAME,
};


int cmd_webp_init(RXIFRM *frm, void *ctx);
int cmd_encode(RXIFRM *frm, void *ctx);
int cmd_decode(RXIFRM *frm, void *ctx);
int cmd_config(RXIFRM *frm, void *ctx);
int cmd_anim_encoder(RXIFRM *frm, void *ctx);
int cmd_encode_frame(RXIFRM *frm, void *ctx);

enum ma_arg_words {W_ARG_0,
	W_ARG_ALPHA_COMPRESSION,
	W_ARG_ALPHA_FILTERING,
	W_ARG_ALPHA_QUALITY,
	W_ARG_AUTOFILTER,
	W_ARG_BACKGROUND,
	W_ARG_EMULATE_JPEG_SIZE,
	W_ARG_EXACT,
	W_ARG_FILTER_SHARPNESS,
	W_ARG_FILTER_STRENGTH,
	W_ARG_HINT,
	W_ARG_LOSSLESS,
	W_ARG_LOW_MEMORY,
	W_ARG_METHOD,
	W_ARG_MULTI_THREADED,
	W_ARG_NEAR_LOSSLESS,
	W_ARG_PARTITION_LIMIT,
	W_ARG_PARTITIONS,
	W_ARG_PASS,
	W_ARG_PREPROCESSING,
	W_ARG_PRESET,
	W_ARG_PSNR,
	W_ARG_QMAX,
	W_ARG_QMIN,
	W_ARG_QUALITY,
	W_ARG_SEGMENTS,
	W_ARG_SHARP_YUV,
	W_ARG_SIZE,
	W_ARG_SNS_STRENGTH,
	W_ARG_STRONG,
	W_ARG_PROGRESS,
	W_ARG_VERBOSE
};
enum ma_type_words {W_TYPE_0,
	W_TYPE_DEFAULT,
	W_TYPE_PICTURE,
	W_TYPE_PHOTO,
	W_TYPE_DRAWING,
	W_TYPE_ICON,
	W_TYPE_TEXT,
	W_HINT_DEFAULT,
	W_HINT_PICTURE,
	W_HINT_PHOTO,
	W_HINT_GRAPH
};

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define WEBP_EXT_INIT_CODE \
	"REBOL [Title: {Rebol WebP Codec Extension} Type: module Version: 1.4.0.1 Needs: 3.14.1 Home:  https://github.com/Oldes/Rebol-WebP ]\n"\
	"webp-init: command [args [block!] presets [block!] hints [block!]]\n"\
	"encode: command [\"Encode an image into WebP format\" image [image!] \"Rebol image to be encoded\"]\n"\
	"decode: command [\"Decode a WebP image into a Rebol image\" data [binary!] \"Binary data containing an encoded WebP image\"]\n"\
	"config: command [{Set codec's parameters. They can be useful to better balance the trade-off between compression efficiency and processing time.} spec [word! block!] {Preset name (photo, picture, drawing, icon, text) or block with parameters}]\n"\
	"anim-encoder: command [\"Initialize a new WebP Image Encoder\" size [pair!] \"Size of the output\"]\n"\
	"encode-frame: command [\"Encode an image into a WebPAnimEncoder object\" encoder [handle!] {WebPAnimEncoder object to which the frame is to be added} time [time!] \"Timestamp of this frame\" image [image! none!] {Rebol image to be added. If none, the animation will be assembled into a binary.}]\n"\
	"\n"\
	"config-presets: [\n"\
	"	default\n"\
	"	picture  ;; digital picture, like portrait, inner shot\n"\
	"	photo    ;; outdoor photograph, with natural lighting\n"\
	"	drawing  ;; hand or line drawing, with high-contrast details\n"\
	"	icon     ;; small-sized colorful images\n"\
	"	text     ;; text-like\n"\
	"]\n"\
	"image-hints: [\n"\
	"	default\n"\
	"	picture  ;; digital picture, like portrait, inner shot\n"\
	"	photo    ;; outdoor photograph, with natural lighting\n"\
	"	graph    ;; discrete tone image (graph, map-tile etc).\n"\
	"]\n"\
	"\n"\
	"config-options: [\n"\
	"	;- Encoder options\n"\
	"    alpha-compression logic!   \"Algorithm for encoding the alpha plane (default is compressed with WebP lossless)\"\n"\
	"    alpha-filtering   integer! \"Predictive filtering method for alpha plane. 0: none, 1: fast, 2: best. Default if 1\"\n"\
	"    alpha-quality     number!  \"Between 0 (smallest size) and 100 (lossless). Default is 100\"\n"\
	"    autofilter        logic!   \"Auto adjust filter's strength\"\n"\
	"    background        tuple!   \"Background color used for alpha blending (or none)\"\n"\
	"    emulate-jpeg-size logic!   \"If true, compression parameters will be remapped to better match the expected output size from JPEG compression. Generally, the output size will be similar but the degradation will be lower\"\n"\
	"    exact             logic!   \"Preserve the exact RGB values under transparent area. Otherwise, discard this invisible RGB information for better compression. The default is off\"\n"\
	"    filter-sharpness  integer! \"Range: [0 = off .. 7 = least sharp]\"\n"\
	"    filter-strength   integer! \"Range: [0 = off .. 100 = strongest]\"\n"\
	"    hint              word!    \"Hint for image type (lossless only for now)\"\n"\
	"    lossless          logic!   \"Lossless encoding\"\n"\
	"    low-memory        logic!   \"If set, reduce memory usage (but increase CPU use)\"\n"\
	"    method            integer! \"Quality/speed trade-off (0=fast, 6=slower-better)\"\n"\
	"    multi-threaded    logic!   \"Try and use multi-threaded encoding\"\n"\
	"    near-lossless     integer! \"Near lossless encoding [0 = max loss .. 100 = off (default)]\"\n"\
	"    partition-limit   integer! \"Quality degradation allowed to fit the 512k limit on prediction modes coding (0: no degradation, 100: maximum possible degradation)\"\n"\
	"    partitions        integer! \"Log2(number of token partitions) in [0..3]. Default is set to 0 for easier progressive decoding\"\n"\
	"    pass              integer! \"Number of entropy-analysis passes (in [1..10])\"\n"\
	"    preprocessing     integer! \"0=none, 1=segment-smooth, 2=pseudo-random dithering\"\n"\
	"    preset            word!    \"One of `config-presets`\"\n"\
	"    psnr              number!  \"If non-zero, specifies the minimal distortion to try to achieve. Takes precedence over target_size.\"\n"\
	"    qmax              integer! \"Maximum permissible quality factor\"\n"\
	"    qmin              integer! \"Minimum permissible quality factor\"\n"\
	"    quality           number!  \"Between 0 and 100\"\n"\
	"    segments          integer! \"Maximum number of segments to use, in [1..4]\"\n"\
	"    sharp-yuv         logic!   \"If needed, use sharp (and slow) RGB->YUV conversion\"\n"\
	"    size              integer! \"If non-zero, set the desired target size in bytes. Takes precedence over the 'compression' parameter.\"\n"\
	"    sns-strength      integer! \"Spatial Noise Shaping. 0=off, 100=maximum.\"\n"\
	"    strong            logic!   \"Filtering type (only used if filter-strength > 0 or autofilter > 0)\"\n"\
	"    \n"\
	"    ;- Output options\n"\
	"    progress          logic!   \"Report encoding progress\"\n"\
	"    verbose           integer! \"Verbosity level (0 = no output 3 = maximum verbosity)\"\n"\
	"]\n"\
	"\n"\
	"webp-init [alpha-compression alpha-filtering alpha-quality autofilter background emulate-jpeg-size exact filter-sharpness filter-strength hint lossless low-memory method multi-threaded near-lossless partition-limit partitions pass preprocessing preset psnr qmax qmin quality segments sharp-yuv size sns-strength strong progress verbose] :config-presets :image-hints\n"\
	"unset 'webp-init\n"\
	"register-codec [\n"\
	"	name:  'webp\n"\
	"	type:  'image\n"\
	"	title: \"Google's image file format with lossless and lossy compression\"\n"\
	"	suffixes: [%.webp]\n"\
	"	decode:  func[data [binary!]][system/modules/webp/decode data]\n"\
	"	encode:  func[data [image! ]][system/modules/webp/encode data]\n"\
	"	identify: func [data [binary!]][parse data [#{52494646} 4 skip #{57454250} to end]]\n"\
	"]\n"

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

#define APPEND_STRING(str, ...) \
	len = snprintf(NULL,0,__VA_ARGS__);\
	if (len > (int)(SERIES_REST(str)-SERIES_LEN(str))) {\
		RL_EXPAND_SERIES(str, SERIES_TAIL(str), len);\
		SERIES_TAIL(str) -= len;\
	}\
	len = snprintf( \
		SERIES_TEXT(str)+SERIES_TAIL(str),\
		SERIES_REST(str)-SERIES_TAIL(str),\
		__VA_ARGS__\
	);\
	SERIES_TAIL(str) += len;

