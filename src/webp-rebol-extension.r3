REBOL [
	title:  "Rebol/WebP module builder"
	type:    module
	date:    31-May-2024
	home:    https://github.com/Oldes/Rebol-WebP
	version: 1.4.0
	author: @Oldes
]

;- all extension command specifications ----------------------------------------
commands: [
	webp-init: [args [block!] presets [block!] hints [block!]] ;; used internaly only!
	encode: [
		"Encode an image into WebP format"
		image [image!] "Rebol image to be encoded"
	]
	decode: [
		"Decode a WebP image into a Rebol image"
		data [binary!] "Binary data containing an encoded WebP image"
	]
	config: [
		"Set codec's parameters. They can be useful to better balance the trade-off between compression efficiency and processing time."
		spec [word! block!] "Preset name (photo, picture, drawing, icon, text) or block with parameters"
	]
]

ext-values: {
config-presets: [
	default
	picture  ;; digital picture, like portrait, inner shot
	photo    ;; outdoor photograph, with natural lighting
	drawing  ;; hand or line drawing, with high-contrast details
	icon     ;; small-sized colorful images
	text     ;; text-like
]
image-hints: [
	default
	picture  ;; digital picture, like portrait, inner shot
	photo    ;; outdoor photograph, with natural lighting
	graph    ;; discrete tone image (graph, map-tile etc).
]

config-options: [
	;- Encoder options
    alpha-compression logic!   "Algorithm for encoding the alpha plane (default is compressed with WebP lossless)"
    alpha-filtering   integer! "Predictive filtering method for alpha plane. 0: none, 1: fast, 2: best. Default if 1"
    alpha-quality     number!  "Between 0 (smallest size) and 100 (lossless). Default is 100"
    autofilter        logic!   "Auto adjust filter's strength"
    background        tuple!   "Background color used for alpha blending (or none)"
    emulate-jpeg-size logic!   "If true, compression parameters will be remapped to better match the expected output size from JPEG compression. Generally, the output size will be similar but the degradation will be lower"
    exact             logic!   "Preserve the exact RGB values under transparent area. Otherwise, discard this invisible RGB information for better compression. The default is off"
    filter-sharpness  integer! "Range: [0 = off .. 7 = least sharp]"
    filter-strength   integer! "Range: [0 = off .. 100 = strongest]"
    hint              word!    "Hint for image type (lossless only for now)"
    lossless          logic!   "Lossless encoding"
    low-memory        logic!   "If set, reduce memory usage (but increase CPU use)"
    method            integer! "Quality/speed trade-off (0=fast, 6=slower-better)"
    multi-threaded    logic!   "Try and use multi-threaded encoding"
    near-lossless     integer! "Near lossless encoding [0 = max loss .. 100 = off (default)]"
    partition-limit   integer! "Quality degradation allowed to fit the 512k limit on prediction modes coding (0: no degradation, 100: maximum possible degradation)"
    partitions        integer! "Log2(number of token partitions) in [0..3]. Default is set to 0 for easier progressive decoding"
    pass              integer! "Number of entropy-analysis passes (in [1..10])"
    preprocessing     integer! "0=none, 1=segment-smooth, 2=pseudo-random dithering"
    preset            word!    "One of `config-presets`"
    psnr              number!  "If non-zero, specifies the minimal distortion to try to achieve. Takes precedence over target_size."
    qmax              integer! "Maximum permissible quality factor"
    qmin              integer! "Minimum permissible quality factor"
    quality           number!  "Between 0 and 100"
    segments          integer! "Maximum number of segments to use, in [1..4]"
    sharp-yuv         logic!   "If needed, use sharp (and slow) RGB->YUV conversion"
    size              integer! "If non-zero, set the desired target size in bytes. Takes precedence over the 'compression' parameter."
    sns-strength      integer! "Spatial Noise Shaping. 0=off, 100=maximum."
    strong            logic!   "Filtering type (only used if filter-strength > 0 or autofilter > 0)"
    
    ;- Output options
    progress          logic!   "Report encoding progress"
    verbose           integer! "Verbosity level (0 = no output 3 = maximum verbosity)"
]
}

words: transcode ext-values

arg-words:   copy [] foreach [a b c] words/config-options [append arg-words a]
type-words:  words/config-presets
hint-words:  words/image-hints

arg-words: unique arg-words

;-------------------------------------- ----------------------------------------
reb-code: ajoin [
	"REBOL [Title: {Rebol WebP Codec Extension} "
	"Type: module "
	"Version: 1.4.0.0 "
	"Needs: 3.14.1 "
	"Home:  https://github.com/Oldes/Rebol-WebP "
	"]"
]
enu-commands:  "" ;; command name enumerations
cmd-declares:  "" ;; command function declarations
cmd-dispatch:  "" ;; command functionm dispatcher

ma-arg-words: "enum ma_arg_words {W_ARG_0"
ma-type-words: "enum ma_type_words {W_TYPE_0"
ma-hint-words: "enum ma_type_words {W_HINT_0"

;- generate C and Rebol code from the command specifications -------------------
foreach [name spec] commands [
	append reb-code ajoin [lf name ": command "]
	new-line/all spec false
	append/only reb-code mold spec

	name: form name
	replace/all name #"-" #"_"
	replace/all name #"?" #"q"
	
	append enu-commands ajoin ["^/^-CMD_WEBP_" uppercase copy name #","]

	append cmd-declares ajoin ["^/int cmd_" name "(RXIFRM *frm, void *ctx);"]
	append cmd-dispatch ajoin ["^-cmd_" name ",^/"]
]

;- additional Rebol initialization code ----------------------------------------

foreach word arg-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	replace/all word #"?" #"Q"
	append ma-arg-words ajoin [",^/^-W_ARG_" word]
]

foreach word type-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append ma-type-words ajoin [",^/^-W_TYPE_" word]
]

foreach word hint-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append ma-type-words ajoin [",^/^-W_HINT_" word]
]

append ma-arg-words "^/};"
append ma-type-words "^/};"
append ma-hint-words "^/};"
append reb-code ajoin [
	LF ext-values {
webp-init } mold/flat arg-words { :config-presets :image-hints
unset 'webp-init
register-codec [
	name:  'webp
	type:  'image
	title: "Google's image file format with lossless and lossy compression"
	suffixes: [%.webp]
	decode:  func[data [binary!]][system/modules/webp/decode data]
	encode:  func[data [image! ]][system/modules/webp/encode data]
	identify: func [data [binary!]][parse data [#{52494646} 4 skip #{57454250} to end]]
]}
]


logo: next {
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
//}
;append reb-code {}

;print reb-code

;- convert Rebol code to C-string ----------------------------------------------
init-code: copy ""
foreach line split reb-code lf [
	replace/all line #"^"" {\"}
	append init-code ajoin [{\^/^-"} line {\n"}] 
]

;-- C file webps -----------------------------------------------------------
header: {$logo
#include "rebol-extension.h"
#include "webp/encode.h"
#include "webp/decode.h"

#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 14
#define MIN_REBOL_UPD 1
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

extern u32* arg_words;
extern u32* type_words;
extern u32* hint_words;

enum ext_commands {$enu-commands
};

$cmd-declares

$ma-arg-words
$ma-type-words

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define WEBP_EXT_INIT_CODE $init-code

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
	if (len > SERIES_REST(str)-SERIES_LEN(str)) {\
		RL_EXPAND_SERIES(str, SERIES_TAIL(str), len);\
		SERIES_TAIL(str) -= len;\
	}\
	len = snprintf( \
		SERIES_TEXT(str)+SERIES_TAIL(str),\
		SERIES_REST(str)-SERIES_TAIL(str),\
		__VA_ARGS__\
	);\
	SERIES_TAIL(str) += len;

}
;;------------------------------------------------------------------------------
ctable: {$logo
#include "webp-rebol-extension.h"
MyCommandPointer Command[] = {
$cmd-dispatch};
}

;- output generated files ------------------------------------------------------
write %webp-rebol-extension.h reword :header self
write %webp-commands-table.c  reword :ctable self



;; README documentation...
doc: clear ""
hdr: clear ""
arg: clear ""
cmd: desc: a: t: s: readme: r: none
parse commands [
	any [
		quote webp-init: skip ;; skip the internal init command
		|
		set cmd: set-word! into [
			(clear hdr clear arg r: none)
			(append hdr ajoin [LF LF "#### `" cmd "`"])
			set desc: opt string!
			any [
				set a word!
				set t opt block!
				set s opt string!
				(
					unless r [append hdr ajoin [" `:" a "`"]]
					append arg ajoin [LF "* `" a "`"] 
					if t [append arg ajoin [" `" mold t "`"]]
					if s [append arg ajoin [" " s]]
				)
				|
				set r refinement!
				set s opt string!
				(
					append arg ajoin [LF "* `/" r "`"] 
					if s [append arg ajoin [" " s]]
				)
			]
			(
				append doc hdr
				append doc LF
				append doc any [desc ""]
				append doc arg
			)
		]
	]
]

try/except [
	readme: read/string %../README.md
	readme: clear find/tail readme "## Extension commands:"
	append readme ajoin [
		LF doc
		LF LF
		LF "## Other extension values:"
		LF "```rebol"
		trim/tail ext-values
		LF "```"
		LF
	]
	write %../README.md head readme
] :print


