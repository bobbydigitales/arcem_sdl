MacOS setup

1. Install xcode command-line tool `xcode-select --install`
2. Install homebrew https://brew.sh/
3. Install SDL2 `brew install sdl2`
4. clone this repo `git clone https://github.com/bobbydigitales/arcem_sdl.git`
5. `cd arcem_sdl`
6. build `SYSTEM=sdl make -j all`
7. run `./arcem`
8. At this point it should run and fail because you don't have an OS ROM file.
