# pixelWind
i dunno, glitch images through scripts


## Compile
linux: `clang main.c -Ofast -flto -march=native -mtune=native -fuse-ld=lld -lm -o main`

windows: `clang main.c -Ofast -flto -march=native -mtune=native -fuse-ld=lld -o main.exe`


### Run
`main inputImage pxlBleed/pxlWind numberOfSteps`

Example: `main img.jpg pxlBleed 20`
