//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/WebP extension
// SPDX-License-Identifier: MIT
// =============================================================================
// Content of this file was partially taken from libwebp examples.

static const char* const kErrorEncMessages[VP8_ENC_ERROR_LAST] = {
  "OK",
  "VP8_ENC_ERROR_OUT_OF_MEMORY",
  "VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY",
  "VP8_ENC_ERROR_NULL_PARAMETER",
  "VP8_ENC_ERROR_INVALID_CONFIGURATION",
  "VP8_ENC_ERROR_BAD_DIMENSION",
  "VP8_ENC_ERROR_PARTITION0_OVERFLOW",
  "VP8_ENC_ERROR_PARTITION_OVERFLOW",
  "VP8_ENC_ERROR_BAD_WRITE",
  "VP8_ENC_ERROR_FILE_TOO_BIG",
  "VP8_ENC_ERROR_USER_ABORT"
};

#define VP8_DEC_ERROR_LAST 8
static const char* const kErrorDecMessages[VP8_DEC_ERROR_LAST] = {
  "OK",
  "VP8_STATUS_OUT_OF_MEMORY",
  "VP8_STATUS_INVALID_PARAM",
  "VP8_STATUS_BITSTREAM_ERROR",
  "VP8_STATUS_UNSUPPORTED_FEATURE",
  "VP8_STATUS_SUSPENDED",
  "VP8_STATUS_USER_ABORT",
  "VP8_STATUS_NOT_ENOUGH_DATA"
};


static void PrintByteCount(const int bytes[4], int total_size,
                           int* const totals) {
  int s;
  int total = 0;
  for (s = 0; s < 4; ++s) {
    fprintf(stderr, "| %7d ", bytes[s]);
    total += bytes[s];
    if (totals) totals[s] += bytes[s];
  }
  fprintf(stderr, "| %7d  (%.1f%%)\n", total, 100.f * total / total_size);
}

static void PrintPercents(const int counts[4]) {
  int s;
  const int total = counts[0] + counts[1] + counts[2] + counts[3];
  for (s = 0; s < 4; ++s) {
    fprintf(stderr, "|     %3d%%", (int)(100. * counts[s] / total + .5));
  }
  fprintf(stderr, "| %7d\n", total);
}

static void PrintValues(const int values[4]) {
  int s;
  for (s = 0; s < 4; ++s) {
    fprintf(stderr, "| %7d ", values[s]);
  }
  fprintf(stderr, "|\n");
}

static void PrintMapInfo(const WebPPicture* const pic) {
  if (pic->extra_info != NULL) {
    const int mb_w = (pic->width + 15) / 16;
    const int mb_h = (pic->height + 15) / 16;
    const int type = pic->extra_info_type;
    int x, y;
    for (y = 0; y < mb_h; ++y) {
      for (x = 0; x < mb_w; ++x) {
        const int c = pic->extra_info[x + y * mb_w];
        if (type == 1) {   // intra4/intra16
          fprintf(stderr, "%c", "+."[c]);
        } else if (type == 2) {    // segments
          fprintf(stderr, "%c", ".-*X"[c]);
        } else if (type == 3) {    // quantizers
          fprintf(stderr, "%.2d ", c);
        } else if (type == 6 || type == 7) {
          fprintf(stderr, "%3d ", c);
        } else {
          fprintf(stderr, "0x%.2x ", c);
        }
      }
      fprintf(stderr, "\n");
    }
  }
}

static void PrintFullLosslessInfo(const WebPAuxStats* const stats,
                  const char* const description) {
  fprintf(stderr, " [WebP] Lossless-%s compressed size: %d bytes\n",
          description, stats->lossless_size);
  fprintf(stderr, "        * Header size: %d bytes, image data size: %d\n",
          stats->lossless_hdr_size, stats->lossless_data_size);
  if (stats->lossless_features) {
    fprintf(stderr, "        * Lossless features used:");
    if (stats->lossless_features & 1) fprintf(stderr, " PREDICTION");
    if (stats->lossless_features & 2) fprintf(stderr, " CROSS-COLOR-TRANSFORM");
    if (stats->lossless_features & 4) fprintf(stderr, " SUBTRACT-GREEN");
    if (stats->lossless_features & 8) fprintf(stderr, " PALETTE");
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "        * Precision Bits: histogram=%d", stats->histogram_bits);
  if (stats->lossless_features & 1) {
    fprintf(stderr, " prediction=%d", stats->transform_bits);
  }
  if (stats->lossless_features & 2) {
    fprintf(stderr, " cross-color=%d", stats->cross_color_transform_bits);
  }
  fprintf(stderr, " cache=%d\n", stats->cache_bits);
  if (stats->palette_size > 0) {
    fprintf(stderr, "        * Palette size:   %d\n", stats->palette_size);
  }
}

static void PrintExtraInfoLossy(const WebPPicture* const pic) {
  const WebPAuxStats* const stats = pic->stats;
  if (verbose_level == 1) {
    fprintf(stderr, "%7d %2.2f\n", stats->coded_size, stats->PSNR[3]);
  } else {
    const int num_i4 = stats->block_count[0];
    const int num_i16 = stats->block_count[1];
    const int num_skip = stats->block_count[2];
    const int total = num_i4 + num_i16;
    fprintf(stderr, " [WebP] Dimension: %d x %d%s\n",
            pic->width, pic->height,
            stats->alpha_data_size ? " (with alpha)" : "");
    fprintf(stderr, " [WebP] Output:    "
            "%d bytes Y-U-V-All-PSNR %2.2f %2.2f %2.2f   %2.2f dB (%.2f bpp)\n",
            stats->coded_size,
            stats->PSNR[0], stats->PSNR[1], stats->PSNR[2], stats->PSNR[3],
            8.f * stats->coded_size / pic->width / pic->height);
    if (total > 0) {
      int totals[4] = { 0, 0, 0, 0 };
      fprintf(stderr, "        block count: intra4:      %6d  (%.2f%%)\n"
              "                     intra16:     %6d  (%.2f%%)\n"
              "                     skipped:     %6d  (%.2f%%)\n",
              num_i4, 100.f * num_i4 / total,
              num_i16, 100.f * num_i16 / total,
              num_skip, 100.f * num_skip / total);
      fprintf(stderr, "        bytes used:  header:      %6d  (%.1f%%)\n"
              "                     partition:   %6d  (%.1f%%)\n",
              stats->header_bytes[0],
              100.f * stats->header_bytes[0] / stats->coded_size,
              stats->header_bytes[1],
              100.f * stats->header_bytes[1] / stats->coded_size);
      if (stats->alpha_data_size > 0) {
        fprintf(stderr, "                     transparency:   %6d (%.1f dB)\n",
                stats->alpha_data_size, stats->PSNR[4]);
      }
      fprintf(stderr, "         Residuals bytes  "
              "|segment 1|segment 2|segment 3"
              "|segment 4|  total\n");
      if (verbose_level > 2) {
        fprintf(stderr, "          intra4-coeffs:  ");
        PrintByteCount(stats->residual_bytes[0], stats->coded_size, totals);
        fprintf(stderr, "         intra16-coeffs:  ");
        PrintByteCount(stats->residual_bytes[1], stats->coded_size, totals);
        fprintf(stderr, "          chroma coeffs:  ");
        PrintByteCount(stats->residual_bytes[2], stats->coded_size, totals);
      }
      fprintf(stderr, "            macroblocks:  ");
      PrintPercents(stats->segment_size);
      fprintf(stderr, "              quantizer:  ");
      PrintValues(stats->segment_quant);
      fprintf(stderr, "           filter level:  ");
      PrintValues(stats->segment_level);
      if (verbose_level > 2) {
        fprintf(stderr, "        ------------------+---------");
        fprintf(stderr, "+---------+---------+---------+-----------------\n");
        fprintf(stderr, "         segments total:  ");
        PrintByteCount(totals, stats->coded_size, NULL);
      }
    }
    if (stats->lossless_size > 0) {
      PrintFullLosslessInfo(stats, "alpha");
    }
  }
}

static void PrintExtraInfoLossless(const WebPPicture* const pic) {
  const WebPAuxStats* const stats = pic->stats;
  if (verbose_level == 1) {
    fprintf(stderr, " [WebP] Output: %7d bytes, PSNR: %2.2f\n", stats->coded_size, stats->PSNR[3]);
  } else {
    fprintf(stderr, " [WebP] Dimension: %d x %d\n", pic->width, pic->height);
    fprintf(stderr, " [WebP] Output:    %d bytes (%.2f bpp)\n", stats->coded_size,
            8.f * stats->coded_size / pic->width / pic->height);
    if (verbose_level > 2)
       PrintFullLosslessInfo(stats, "ARGB");
  }
}

// -----------------------------------------------------------------------------
// Metadata writing.
/*
enum {
  METADATA_EXIF = (1 << 0),
  METADATA_ICC  = (1 << 1),
  METADATA_XMP  = (1 << 2),
  METADATA_ALL  = METADATA_EXIF | METADATA_ICC | METADATA_XMP
};

static const int kChunkHeaderSize = 8;
static const int kTagSize = 4;

static void PrintMetadataInfo(const Metadata* const metadata, int metadata_written) {
  if (metadata == NULL || metadata_written == 0) return;

  fprintf(stderr, "Metadata:\n");
  if (metadata_written & METADATA_ICC) {
    fprintf(stderr, "  * ICC profile:  %6d bytes\n", (int)metadata->iccp.size);
  }
  if (metadata_written & METADATA_EXIF) {
    fprintf(stderr, "  * EXIF data:    %6d bytes\n", (int)metadata->exif.size);
  }
  if (metadata_written & METADATA_XMP) {
    fprintf(stderr, "  * XMP data:     %6d bytes\n", (int)metadata->xmp.size);
  }
}
*/

static int ProgressReport(int percent, const WebPPicture* const picture) {
  fprintf(stderr, " [WebP] Encoding image: %3d %%      \r", percent);
  return 1;  // all ok
}
