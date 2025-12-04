# Screenshot → ASCII Art (C)

Simple, fast command-line tool that converts images into monochrome ASCII art.

Build
```bash
# compile using this or use compiled version i also push mine
gcc -O2 ascii_art.c -o ascii_art -lm
```

Usage
```bash
./ascii_art <image> [width] [inv]
# <image> : path to image
# width   : output width in characters (default 120)
# inv     : optional literal "inv" to invert dark/light mapping
```

Examples
```bash
./ascii_art photo.jpg
./ascii_art photo.jpg 80
./ascii_art photo.jpg 100 inv > art.txt
```
<img width="1720" height="959" alt="image" src="https://github.com/user-attachments/assets/308643d1-f11a-4e32-bc4d-c892807f7c19" />

