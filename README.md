# MangaDownloader
Downlods a manga from mangadex using their api


# Usage
```cmd
 pathToProgram -i MangaID -o outputDir (optinal)
```
## Other arguments
 -m : the configuration the manga will be downloded in options are manga , volume (defualt) ,chapter
 -dt : quality of the files being used , options are data (fullquality which is the defualt) or datasaver (reduced quality saving space)

# Libarires used
 - cmdParser: https://github.com/SuperDuperBlade/cmdParser
 - cpp-httplib: https://github.com/yhirose/cpp-httplib
 - simdjson: https://github.com/simdjson/simdjson
 - libzip: https://libzip.org/