<div align="center">
    <img src="./examples/bleedlogo2.png" width="750" height="300">
    <h1> pixelWind </h1>
    <p>
        <b>Edgy image processing 😎</b>
    </p>
    <br>    
    A tool to apply fun pixel-sorting-esque effects to images.
    <br>
    <br>
    <h2>Examples 👇</h2>
</div>





# Usage

Example Usage: `./main img.png -O 4 -y -o cool.gif`

`img.png` - Input image

`-O 4` - Maximum pixel offset of 4

`-y` - Only offset y axis

`-o cool.gif` - Output file cool.gif

<br>

Options:

  -I iterations: Number of frames to generate (default 50)

  -O offset: Maximum pixel offset (default 2)

  -t tolerance: Minimum brightness to trigger effect (default 128)

  -f frame_rate: Frame rate of output gif (default 20)

  -r randChance: Chance of effect happening (default 10)

  -m mode: Effect mode bleed/diffuse/wind/haze (default bleed)

  -x: Only offset x axis

  -y: Only offset y axis

  -o output: Output file (default output.gif)

  -i input: Input file

  -h: Show help

  image: Path to image
