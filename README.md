# pixelWind
i dunno, glitch images through scripts


![example](https://github.com/zigzag1001/pixelWind/blob/main/out/output1.gif)


## Compile
linux: `clang main.c -Ofast -flto -march=native -mtune=native -fuse-ld=lld -lm -o main`

windows: `clang main.c -Ofast -flto -march=native -mtune=native -fuse-ld=lld -o main.exe`


### Run
`main inputImage pxlBleed/pxlWind numberOfSteps`

Example: `main img.jpg pxlBleed 20`
