Rebol [
    title: "Rebol/WebP extension CI test"
]

print ["Running test on Rebol build:" mold to-block system/build]

system/options/quiet: false
system/options/log/rebol: 4

CI?: any [
    "true" = get-env "CI"
    "true" = get-env "GITHUB_ACTIONS"
    "true" = get-env "TRAVIS"
    "true" = get-env "CIRCLECI"
    "true" = get-env "GITLAB_CI"
]

if CI? [
    ;; for the CI test the module is the build directory 
    system/options/modules: dirize to-real-file %build/ ;@@ to-real-file on linux does not include the tailing slash!
]

;; make sure that we load a fresh extension
try [system/modules/webp: none]

webp: import webp
? webp

;; do some test with the extension

test-animation: function [][
    random/seed 2
    size: 480x480 ;; output size
    center: size / 2

    enc: webp/anim-encoder size
    if not handle? enc [
        print as-purple "Failed to initialize WebPAnimEncoder handle!"
        return false
    ]

    ;; import Blend2D extension used for drawing...
    import blend2d


    ;; destination image...
    img: make image! size
    fish: premultiply load %fish.png
    fish-size: fish/size

    frames: 30
    frame-time: 0:0:0
    time-increment: 0.1

    background: 255.255.255
    offset: center - (fish-size / 2) - 25x25

    loop frames [
        ;; draw some content...
        pos: offset + random 50x50
        draw img [
            ;; clear all with the background color
            fill :background fill-all
            ;; draw an image
            image :fish :pos
        ]
        ;; add frame to the animation...
        webp/encode-frame :enc :frame-time img
        ;; update time ...
        frame-time: frame-time + time-increment
    ]
    ;; save the final animation...
    write %anim.webp webp/encode-frame :enc :frame-time none
]

unless all [
    print as-yellow "Loading PNG image using a codec..."
    probe image?  try [img: load %fish.png]

    print as-yellow "Using configuration with a preset name..."
    probe webp/config 'photo

    print as-yellow "Using configuration with a block with options..."
    probe webp/config [
        preset:    'default ;; this resets everything
        lossless:   8
        verbose:    3
        progress:   off
    ]
    print as-yellow "Encode image to WebP binary using the native command..."
    probe binary? try [bin: webp/encode img]
    print as-yellow "Write the binary into a file..."
    probe file?   try [out: write %out.webp bin]

    print as-yellow "Change configuration to use lossy output..."
    probe webp/config [preset: 'default quality: 50]
    print as-yellow "Encode image using a codec..."
    probe file?   try [save %out-lossy.webp img]

    print as-yellow "Decoding the binary using the native command..."
    probe image?  try [img: webp/decode bin]
    print as-yellow "Loading WebP image using a codec..."
    probe image?  try [img: load %out.webp]
    
    print as-yellow "Test WebPAnimEncoder..."
    probe file?   try [test-animation]
][
    print as-purple "Some test failed!"
    quit/return 1
]


print as-green "DONE"

