# assfonts

Subset fonts and embed them into an ASS subtitle.

(Update v0.2.0 Add GUI version)

### Build Requirements

Build environment ---- Conan version 1.56.0

- [FreeType](http://freetype.org/)

- [HarfBuzz](https://github.com/harfbuzz/harfbuzz)

- [Boost](https://www.boost.org/) filesystem serialization program_options

- [libiconv](https://www.gnu.org/software/libiconv/)

- [spdlog](https://github.com/gabime/spdlog)

- [fmt](https://github.com/fmtlib/fmt)

- [wxWidgets](https://www.wxwidgets.org/)

### How to build

```
conan install . -b=missing -s build_type=Release
conan build .
```

### How to use

```
Usage:     assfonts [options...] [<file>]
Examples:  assfonts <file>                  Embed subset fonts into ASS script
           assfonts -i <file>               Same as above
           assfonts -o <dir> -s -i <file>   Only subset fonts but not embed
           assfonts -f <dir> -e -i <file>   Only embed fonts without subset
           assfonts -f <dir> -b             Build or update fonts database only
Options:
  -i, --input,      <file>   Input .ass file
  -o, --output      <dir>    Output directory  (Default: same directory as input)
  -f, --fontpath    <dir>    Set fonts directory
  -b, --build                Build or update fonts database  (Require --fontpath)
  -d, --dbpath      <dir>    Set fonts database path  (Default: current path)
  -s, --subset-only <bool>   Subset fonts but not embed them into subtitle  (default: False)
  -e, --embed-only  <bool>   Do not subset fonts  (default: False)
  -v, --verbose     <num>    Set logging level (0 to 3), 0 is off  (Default: 3)
  -h, --help                 Get help info
 ```
 
 When you first run this program, it's recommended to initialize font database first `assfonts -f <your_fonts_dir> -b` 
 If database path is not specified, it will be saved in `<your_current_working_directory>/fonts.db`
 
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
 
