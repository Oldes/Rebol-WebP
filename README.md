[![Rebol-WebP CI](https://github.com/Oldes/Rebol-WebP/actions/workflows/main.yml/badge.svg)](https://github.com/Oldes/Rebol-WebP/actions/workflows/main.yml)

# Rebol/WebP

WebP extension for [Rebol3](https://github.com/Oldes/Rebol3) (version 3.14.1 and higher)

## Usage
```rebol
webp: import webp
webp/config 'drawing               ;; use preset for drawing image
webp/config [verbose: 3]           ;; maximum output verbosity
save %out.webp make image! 640x480 ;; save some image into a file encoded using WebP codec
```

## Extension commands:


#### `webp-init` `:args` `:presets` `:hints`

* `args` `[block!]`
* `presets` `[block!]`
* `hints` `[block!]`

#### `encode` `:image`
Encode an image into WebP format
* `image` `[image!]` Rebol image to be encoded

#### `decode` `:data`
Decode a WebP image into a Rebol image
* `data` `[binary!]` Binary data containing an encoded WebP image

#### `config` `:spec`
Set codec's parameters. They can be useful to better balance the trade-off between compression efficiency and processing time.
* `spec` `[word! block!]` Preset name (photo, picture, drawing, icon, text) or block with parameters


## Other extension values:
```rebol
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
```
