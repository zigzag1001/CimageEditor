![output](https://user-images.githubusercontent.com/72932714/235327171-29bee873-2f3e-4d81-ab47-c9eecd91f1e1.gif)


# pixelWind
i dunno, glitch images through scripts


![pxlWind](https://github.com/zigzag1001/pixelWind/blob/main/out/output1.gif)

pxlWind

![pxlBleed](https://github.com/zigzag1001/pixelWind/blob/main/out/output5.gif)

pxlBleed

## Compile
linux: `clang main.c -Ofast -flto -march=native -mtune=native -fuse-ld=lld -lm -o main`

windows: `clang main.c -Ofast -flto -march=native -mtune=native -fuse-ld=lld -o main.exe`


### Run
`main inputImage pxlBleed/pxlWind numberOfSteps`

Example: `main img.jpg pxlBleed 20`
