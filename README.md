# Context Menu Options Attacher (windows)

Looks for .bat files inside folders located inside scripts\ folder and automatically adds a shortcut to the context menu through a .reg file.

``` cmd
├───scripts
│   └───Text Replace
│           text_replace.bat
│           text_replace.ico
│           text_replace.py
```

## Installation

Inside the root folder of the repo execute the following command to generate an .exe

``` cmd
make
```

Maybe the console outputs some warnings, do what ever you want with them. After that you'll find a "install.exe", execute it and all the .bat files will be added automatically to the context menu.