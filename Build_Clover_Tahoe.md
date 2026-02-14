## Build Clover on macOS Tahoe 26 from scratch. 

- Install Xcode 26 or Xcode 16, Make sure you open Xcode then Install additional required components.
- Install [Python 3.13.7](https://www.python.org/ftp/python/3.13.7/python-3.13.7-macos11.pkg) or later
- sudo port install ncurses

### [🍀] ⬇︎ `Clone and Build Clover`
```bash
rm -rf ~/src
mkdir -p ~/src
cd ~/src
git clone https://github.com/CloverHackyColor/CloverBootloader.git
cd ~/src/CloverBootloader && ./buildme
```

### buildme Option:⬇︎
1) build Clover (Default Toolchain)
5) make pkg
