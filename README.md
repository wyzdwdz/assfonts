![Supported Platforms](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue.svg)
[![License](https://img.shields.io/github/license/wyzdwdz/assfonts)](https://github.com/wyzdwdz/assfonts/blob/main/LICENSE)
![Latest Release Tag](https://img.shields.io/github/tag/wyzdwdz/assfonts.svg)

# assfonts

[中文说明](https://bbs.acgrip.com/thread-9897-1-1.html)

Subset fonts and embed them into an ASS subtitle.

### Description

The **assfonts** can search for the required fonts of an ASS subtitle file and subset them. 
It can also embed the UUEncode-encoded font files directly into a subtitle script (which is
based on one feature of ASS subtitle format). In this way, only one subtitle file can contain
all the information to be rendered and achieve the similar effect as those graphical subtitles
such as PGS.

### Build Requirements

- CMake >= 3.16

### How to build

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### How to use

```
Usage:     assfonts [options...] [<files>]
Examples:  assfonts <files>                  Embed subset fonts into ASS script
           assfonts -i <files>               Same as above
           assfonts -o <dir> -s -i <files>   Only subset fonts but not embed
           assfonts -f <dirs> -e -i <files>  Only embed fonts without subset
           assfonts -f <dirs> -b             Build or update fonts database only
           assfonts -l <num> -i <files>      Recolorize the subtitle for HDR contents
Options:
  -i, --input         <files>   Input .ass files
  -o, --output        <dir>     Output directory  (Default: same directory as input)
  -f, --fontpath      <dirs>    Set fonts directories
  -b, --build                   Build or update fonts database  (Require: --fontpath)
  -d, --dbpath        <dir>     Set fonts database path  (Default: current path)
  -s, --subset-only   <bool>    Subset fonts but not embed them into subtitle  (Default: False)
  -e, --embed-only    <bool>    Embed fonts into subtitle but not subset them (Default: False)
  -r, --rename        <bool>    Rename subsetted fonts (Default: False)
  -l, --luminance     <num>     Set subtitle brightness for HDR contents  (Default: 203)
  -m, --multi-thread  <num>     Enable multi thread mode, <num> is the number of threads for processing
                                (Default: <cpu_count> + 1)
  -c, --font-combined <bool>    !!Experimental!! When there are multiple input files, combine the
                                (subsetted) fonts with the same fontname together (Default: False)
  -v, --verbose       <num>     Set logging level (0 to 3), 0 is off  (Default: 3)
  -h, --help                    Get help info
 ```
 
 When you first run this program, it's recommended to initialize font database first `assfonts -f <your_fonts_dir> -b` 
 If database path is not specified, it will be saved in `<your_current_working_directory>/fonts.json`
 
 **Caution!** According [ASS Specs](http://moodub.free.fr/video/ass-specs.doc), only Truetype fonts can be embedded into ASS Script. 
 This program will ignore this rule and embed non-ttf fonts in by force. Some video players may not load these fonts correctly.
 One recommended solution is to subset fonts without embedding firstly by giving option `-s` or `--subset-only` Then convert those
 subsetted fonts (which are located in `<output_directory>/<input_file_name>_subsetted/`) to .ttf fonts using some 3rdparty tools like
 [otf2ttf](https://github.com/shimarulin/otf2ttf). Finally, rerun this program by giving option `-e` or `--embed-only` and setting
 fonts directory to subsetted fonts `-f <output_directory>/<input_file_name>_subsetted/`
 
 ```
 assfonts -s -i <ass_file>
 otf2ttf <ass_file>_subsetted/*.otf
 assfonts -n -f <ass_file>_subsetted/ -i <ass_file>
 ```
