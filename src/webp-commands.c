//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/WebP extension
// SPDX-License-Identifier: MIT
// =============================================================================

#include "webp-rebol-extension.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stopwatch.h"

// Globals.... /////////////////////////////////////////////////////////////////
WebPConfig config;
int show_progress = 0;
int verbose_level = 1;
int blend_alpha = 0;
uint32_t background_color = 0xffffffu;
Stopwatch stop_watch;
////////////////////////////////////////////////////////////////////////////////
#include "webp-output.inc"

#define COMMAND int
#define FRM_IS_HANDLE(n, t)     (RXA_TYPE(frm,n) == RXT_HANDLE && RXA_HANDLE_TYPE(frm, n) == t && IS_USED_HOB(RXA_HANDLE_CONTEXT(frm, n)))
#define ARG_Is_None(n)          (RXA_TYPE(frm,n) == RXT_NONE)
#define ARG_Is_Word(n)          (RXA_TYPE(frm,n) == RXT_WORD)
#define ARG_Is_WebPAnimEncoder(n) FRM_IS_HANDLE(n, Handle_WebPAnimEncoder)
#define ARG_WebPAnimEncoder(n)    ((WebPAnimEncoderWrapper*)(RXA_HANDLE_CONTEXT(frm, n)->handle))->encoder
#define RETURN_ERROR(err)  do {RXA_SERIES(frm, 1) = err; return RXR_ERROR;} while(0)

#ifndef MIN
#ifdef min
#define MIN(a,b) min(a,b)
#define MAX(a,b) max(a,b)
#else
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#endif


int Common_mold(REBHOB *hob, REBSER *str) {
	int len;
	if (!str) return 0;
	SERIES_TAIL(str) = 0;
	APPEND_STRING(str, "0#%lx", (unsigned long)(uintptr_t)hob->data);
	return len;
}


int WebPAnimEncoder_free(void* hndl) {
	REBHOB *hob;
	WebPAnimEncoder *enc;
	if (!hndl) return 0;
	hob = (REBHOB *)hndl;
	if (!hob->data) return 0;
	enc = ((WebPAnimEncoderWrapper*)hob->data)->encoder;
	if (!enc) return 0;
	WebPAnimEncoderDelete(enc);
	UNMARK_HOB(hob);
	return 0;
}

int WebPAnimEncoder_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	//TODO....
	return PE_BAD_SELECT;
}
int WebPAnimEncoder_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	return PE_BAD_SET;
}


COMMAND cmd_webp_init(RXIFRM *frm, void *ctx) {
	arg_words  = RL_MAP_WORDS(RXA_SERIES(frm,1));
	type_words = RL_MAP_WORDS(RXA_SERIES(frm,2));
	hint_words = RL_MAP_WORDS(RXA_SERIES(frm,3));
	// custom initialization may be done here...
	if (!WebPConfigPreset(&config, WEBP_PRESET_DEFAULT, 75)) {
		RETURN_ERROR("WebPConfigPreset failed");
	}
	return RXR_TRUE;
}

COMMAND cmd_encode(RXIFRM *frm, void *ctx) {
	WebPPicture picture;
	WebPPicture original_picture;    // when PSNR or SSIM is requested
	WebPAuxStats stats;
	WebPMemoryWriter memory_writer;
	//Metadata metadata;

	REBSER *ser = (REBSER *)RXA_ARG(frm,1).image;

	//MetadataInit(&metadata);
	WebPMemoryWriterInit(&memory_writer);

	if (!WebPPictureInit(&picture)
	 || !WebPPictureInit(&original_picture)) {
		goto Error;
	}

	picture.use_argb = 1;
	picture.argb   = (uint32_t*)SERIES_DATA(ser);
	picture.width  = RXA_IMAGE_WIDTH(frm, 1);
	picture.height = RXA_IMAGE_HEIGHT(frm, 1);
	picture.argb_stride = picture.width; // * sizeof(uint32_t);

//	debug_print("Image size: %ix%i stride: %u\n", picture.width, picture.height, picture.argb_stride);
//	debug_print("Target quality: %.2f method: %i hint: %i "
//		"segments: %i alpha_compression: %i\n",
//		 config.quality, config.method, config.image_hint, config.segments, config.alpha_compression);

	picture.progress_hook = (show_progress && verbose_level > 0) ? ProgressReport : NULL;

	if (blend_alpha) {
		WebPBlendAlpha(&picture, background_color);
	}

	picture.writer = WebPMemoryWrite;
	picture.custom_ptr = (void*)&memory_writer;

	if (verbose_level > 0) {
		picture.stats = &stats;
	}

	// Compress.
	if (verbose_level > 2) {
		StopwatchReset(&stop_watch);
	}
	if (!WebPEncode(&config, &picture)) goto Error;

	if (verbose_level > 2) {
		const double encode_time = StopwatchReadAndReset(&stop_watch);
		fprintf(stderr, " [WebP] Time to encode picture: %.3fs\n", encode_time);
	}

	if (verbose_level > 0) {
		if (config.lossless) {
			PrintExtraInfoLossless(&picture);
		} else {
			PrintExtraInfoLossy(&picture);
		}
	}
	if (verbose_level > 1 && picture.extra_info_type > 0) {
		PrintMapInfo(&picture);
	}
	/*
	if (print_distortion >= 0) {    // print distortion
		static const char* distortion_names[] = { "PSNR", "SSIM", "LSIM" };
		float values[5];
		if (!WebPPictureDistortion(&picture, &original_picture,
															 print_distortion, values)) {
			fprintf(stderr, "Error while computing the distortion.\n");
			goto Error;
		}
		if (!short_output) {
			fprintf(stderr, "%s: ", distortion_names[print_distortion]);
			fprintf(stderr, "B:%.2f G:%.2f R:%.2f A:%.2f  Total:%.2f\n",
							values[0], values[1], values[2], values[3], values[4]);
		} else {
			fprintf(stderr, "%7d %.4f\n", picture.stats->coded_size, values[4]);
		}
	}
	if (!short_output) {
		PrintMetadataInfo(&metadata, metadata_written);
	}
	*/

	ser = (REBSER *)RL_MAKE_STRING((REBLEN)memory_writer.size, FALSE);
	memcpy(ser->data, memory_writer.mem, memory_writer.size);
	RXA_TYPE(frm, 1) = RXT_BINARY;
	RXA_ARG(frm, 1).series  = ser;
	RXA_ARG(frm, 1).index = 0;
	SERIES_TAIL(ser) = (REBLEN)memory_writer.size;
	return RXR_VALUE;

Error:
	if (picture.error_code > 0) {
		RXA_SERIES(frm, 1) = (void*)(picture.error_code >= VP8_ENC_ERROR_LAST ? "unknown" : kErrorEncMessages[picture.error_code]);
	}
	WebPMemoryWriterClear(&memory_writer);
	WebPFree(picture.extra_info);
	//MetadataFree(&metadata);
	WebPPictureFree(&picture);
	WebPPictureFree(&original_picture);
	return RXR_FALSE;
}

COMMAND cmd_decode(RXIFRM *frm, void *ctx) {
	VP8StatusCode status = VP8_STATUS_OK;
	REBSER *ser = RXA_ARG(frm, 1).series;

	WebPDecoderConfig config;

	if (!WebPInitDecoderConfig(&config)) {
		RXA_SERIES(frm, 1) = "WebPInitDecoderConfig failed!";
		return RXR_ERROR;
	}
	status = WebPGetFeatures(ser->data, SERIES_TAIL(ser), &config.input);
	if (status != VP8_STATUS_OK) goto ErrorStatus;

	// Allocate Rebol image using the reported image size
	REBSER *bin = (REBSER *)RL_MAKE_IMAGE(config.input.width, config.input.height);
	SERIES_TAIL(bin) = config.input.width * config.input.height;
	RXA_TYPE(frm, 1) = RXT_IMAGE;
	RXA_ARG(frm, 1).image = bin;
	RXA_ARG(frm, 1).width  = config.input.width;
	RXA_ARG(frm, 1).height = config.input.height;

	// Use its data as the external memory
	config.output.colorspace = MODE_BGRA;
	config.output.u.RGBA.rgba = (uint8_t*)bin->data;
	config.output.u.RGBA.stride = config.input.width * 4;
	config.output.u.RGBA.size = SERIES_REST(bin) * 4;
	config.output.is_external_memory = 1;

	if (verbose_level > 2) {
		StopwatchReset(&stop_watch);
	}
	// Try to decode the image
	status = WebPDecode(ser->data, SERIES_REST(bin) * 4, &config);
	if (status != VP8_STATUS_OK) goto ErrorStatus;

	if (verbose_level > 2) {
		const double decode_time = StopwatchReadAndReset(&stop_watch);
		fprintf(stderr, " [WebP] Time to decode picture: %.3fs\n", decode_time);
	}

	return RXR_VALUE;

	// Report an error if any
ErrorStatus:
	RXA_SERIES(frm, 1) = (void*)(status >= VP8_DEC_ERROR_LAST ? "unknown" : kErrorDecMessages[status]);
	return RXR_ERROR;
}

static REBOOL fetch_word(REBSER *cmds, REBCNT index, u32* words, REBCNT *cmd) {
	RXIARG arg;
	REBCNT type = RL_GET_VALUE(cmds, index, &arg);
	//debug_print("fetch_word: %u type: %u\n", index, type);
	return ((RXT_WORD == type || RXT_SET_WORD == type) && (cmd[0] = RL_FIND_WORD(words, arg.int32a)));
}


COMMAND cmd_config(RXIFRM *frm, void *ctx) {
	REBCNT word;

	WebPPreset preset = 0;

	if (ARG_Is_Word(1)) {
		preset = (WebPPreset)RL_FIND_WORD(type_words, RXA_WORD(frm, 1));
		//debug_print(" [WebP] Using preset: %u\n", preset);
		if (!WebPConfigPreset(&config, preset, 75)) return RXR_NONE;
	}
	else {
		// block...
		REBSER *args = RXA_SERIES(frm, 1);
		REBCNT index = RXA_INDEX(frm, 1);
		REBCNT type, option;
		RXIARG arg;
		double dec;

		while (index < args->tail) {
			if (!fetch_word(args, index++, arg_words, &option)) {
				debug_print(" [WebP] Unknown option!");
				continue;
			}
			type = RL_GET_VALUE_RESOLVED(args, index++, &arg);
			//debug_print(" [WebP] option: %u type: %u\n", option, type);
			switch(option) {
			case W_ARG_PRESET:
				if (type == RXT_WORD || type == RXT_LIT_WORD) {
					preset = (WebPPreset)RL_FIND_WORD(type_words, arg.int32a);
					if (preset) {
						WebPConfigPreset(&config, preset, 75);
						continue;
					}
				}
				goto Invalid_Option;

			case W_ARG_QUALITY:
				if (type == RXT_DECIMAL || type == RXT_PERCENT) dec = arg.dec64;
				else if (type == RXT_INTEGER) dec = (double)arg.int64;
				else goto Invalid_Option;
				config.quality = (float)dec;
				break;

			case W_ARG_LOSSLESS:
				if (type == RXT_LOGIC)
					config.lossless = arg.int32a;
				else if (type == RXT_INTEGER)
					WebPConfigLosslessPreset(&config, (int)MIN(MAX(0, arg.int64), 9));
				else goto Invalid_Option;
				break;

			case W_ARG_METHOD:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.method = (int)MIN(MAX(0, arg.int64), 6);
				break;

			case W_ARG_HINT:
				if (type == RXT_WORD || type == RXT_LIT_WORD) {
					word = RL_FIND_WORD(hint_words, arg.int32a);
					if(word) {
						config.image_hint = word-1;
						continue;
					}
				}
				goto Invalid_Option;

			case W_ARG_SIZE:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.target_size = (int)MAX(0, arg.int64);
				break;

			case W_ARG_PSNR:
				if (type == RXT_DECIMAL || type == RXT_PERCENT) dec = arg.dec64;
				else if (type == RXT_INTEGER) dec = (double)arg.int64;
				else goto Invalid_Option;
				config.target_PSNR = (float)dec;
				break;

			case W_ARG_SEGMENTS:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.segments = (int)MIN(MAX(1, arg.int64), 4);
				break;

			case W_ARG_SNS_STRENGTH:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.sns_strength = (int)MIN(MAX(0, arg.int64), 100);
				break;

			case W_ARG_FILTER_STRENGTH:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.filter_strength = (int)MIN(MAX(0, arg.int64), 100);
				break;

			case W_ARG_FILTER_SHARPNESS:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.filter_sharpness = (int)MIN(MAX(0, arg.int64), 7);
				break;

			case W_ARG_STRONG:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.filter_type = arg.int32a;
				break;

			case W_ARG_AUTOFILTER:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.autofilter = arg.int32a;
				break;

			case W_ARG_ALPHA_COMPRESSION:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.alpha_compression = arg.int32a;
				break;

			case W_ARG_ALPHA_FILTERING:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.alpha_filtering = (int)MIN(MAX(0, arg.int64), 2);
				break;

			case W_ARG_ALPHA_QUALITY:
				if (type == RXT_INTEGER) dec = (double)arg.int64;
				else if (type == RXT_DECIMAL || type == RXT_PERCENT) dec = arg.dec64;
				else goto Invalid_Option;
				config.alpha_quality = (int)MIN(MAX(0, (int)dec), 100);
				break;

			case W_ARG_PASS:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.pass = (int)MIN(MAX(1, arg.int64), 10);
				break;

			case W_ARG_PREPROCESSING:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.preprocessing = (int)MIN(MAX(0, arg.int64), 2);
				break;

			case W_ARG_PARTITIONS:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.partitions = (int)MIN(MAX(0, arg.int64), 3);
				break;

			case W_ARG_PARTITION_LIMIT:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.partition_limit = (int)MIN(MAX(0, arg.int64), 100);
				break;

			case W_ARG_EMULATE_JPEG_SIZE:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.emulate_jpeg_size = arg.int32a;
				break;

			case W_ARG_MULTI_THREADED:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.thread_level = arg.int32a;
				break;

			case W_ARG_LOW_MEMORY:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.low_memory = arg.int32a;
				break;

			case W_ARG_NEAR_LOSSLESS:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.near_lossless = (int)MIN(MAX(0, arg.int64), 100);
				break;

			case W_ARG_EXACT:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.exact = arg.int32a;
				break;

			case W_ARG_SHARP_YUV:
				if (type != RXT_LOGIC) goto Invalid_Option;
				config.use_sharp_yuv = arg.int32a;
				break;

			case W_ARG_QMIN:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.qmin = (int)MIN(MAX(0, arg.int64), 100);
				break;

			case W_ARG_QMAX:
				if (type != RXT_INTEGER) goto Invalid_Option;
				config.qmax = (int)MIN(MAX(0, arg.int64), 100);
				break;

			case W_ARG_BACKGROUND:
				if (type == RXT_NONE) blend_alpha = 0;
				else if (type == RXT_LOGIC) blend_alpha = arg.int32a;
				else if (type == RXT_TUPLE) {
					blend_alpha = 1;
					background_color = arg.bytes[1] << 16 | arg.bytes[2] << 8 | arg.bytes[3];
				}
				else goto Invalid_Option;
				break;

			case W_ARG_PROGRESS:
				if (type != RXT_LOGIC) goto Invalid_Option;
				show_progress = arg.int32a;
				break;

			case W_ARG_VERBOSE:
				if (type == RXT_INTEGER)
					verbose_level = (int)MIN(MAX(0, arg.int64), 3);
				else if (type == RXT_LOGIC)
					verbose_level = arg.int32a;
				else goto Invalid_Option;
				break;
			}
			continue;
			Invalid_Option:
			debug_print(" [WebP] Invalid option value!\n");
			continue;
		}
	}

	if (!WebPValidateConfig(&config)) return RXR_NONE;

	// Check for unsupported command line options for lossless mode and log
	// warning for such options.
	if (config.lossless == 1) {
		if (config.target_size > 0 || config.target_PSNR > 0) {
			fprintf(stderr, "Encoding for specified size or PSNR is not supported"
							" for lossless encoding. Ignoring such option(s)!\n");
		}
		if (config.partition_limit > 0) {
			fprintf(stderr, "Partition limit option is not required for lossless"
							" encoding. Ignoring this option!\n");
		}
	}

	return RXR_TRUE;
}


COMMAND cmd_anim_encoder(RXIFRM *frm, void *ctx) {
	REBHOB* hob = NULL;
	WebPAnimEncoderOptions enc_options;
	WebPAnimEncoder* enc;
	int width  = (int)RXA_PAIR(frm,1).x;
	int height = (int)RXA_PAIR(frm,1).y;

	WebPAnimEncoderOptionsInit(&enc_options);

	// The encoder is allocated from the WebP library!
	enc = WebPAnimEncoderNew(width, height, &enc_options);
	if (!enc) return RXR_NONE;

	hob = RL_MAKE_HANDLE_CONTEXT(Handle_WebPAnimEncoder);
	if (hob == NULL) return RXR_NONE;
	// So it must be storred in a wrapper (so Rebol don't try to free it)
	((WebPAnimEncoderWrapper*)hob->data)->encoder = enc;

	RXA_HANDLE(frm, 1)       = hob;
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;

	return RXR_VALUE;
}

COMMAND cmd_encode_frame(RXIFRM *frm, void *ctx) {
	WebPAnimEncoder* enc;
	WebPPicture frame;

	if (!ARG_Is_WebPAnimEncoder(1))
		RETURN_ERROR("Expected handle of type: WebPAnimEncoder");

	enc = ARG_WebPAnimEncoder(1);
	int time = (int)(RXA_TIME(frm,2) / 1000000L);
	
	if (ARG_Is_None(3)) {
		WebPData webp_data;

		if (!WebPAnimEncoderAdd(enc, NULL, time, &config)) goto Error;

		WebPAnimEncoderAssemble(enc, &webp_data);

		REBSER *ser = (REBSER *)RL_MAKE_STRING((REBLEN)webp_data.size, FALSE);
		memcpy(ser->data, webp_data.bytes, webp_data.size);
		RXA_TYPE(frm, 1) = RXT_BINARY;
		RXA_ARG(frm, 1).series  = ser;
		RXA_ARG(frm, 1).index = 0;
		SERIES_TAIL(ser) = (REBLEN)webp_data.size;
		return RXR_VALUE;
	}
	
	WebPPictureInit(&frame);

	frame.use_argb = 1;
	frame.argb   = (uint32_t*)SERIES_DATA((REBSER *)RXA_ARG(frm,3).image);
	frame.width  = RXA_IMAGE_WIDTH(frm, 3);
	frame.height = RXA_IMAGE_HEIGHT(frm, 3);
	frame.argb_stride = frame.width;

	if (!WebPAnimEncoderAdd(enc, &frame, time, &config)) goto Error;
 
	return RXR_TRUE;
Error:
	RXA_SERIES(frm, 1) = (void*)WebPAnimEncoderGetError(enc);
	return RXR_ERROR;
}