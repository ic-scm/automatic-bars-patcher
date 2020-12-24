## Nintendo Switch homebrew app

Nintendo Switch homebrew app for running the automatic BARS patcher directly on your Nintendo Switch.

## Configuration

The program stores all the game entries and settings in /switch/auto-bars-patcher/game-config.txt.

A new game entry can be added to the file like this (the lines starting with # are not necessary):

```
# This should be a short identificator name of the game. The ^ is required. For example: acnh or smm2.
^gameid

# This should be the full title name of the game.
full_name=A New Game!

# This is the Original BARS file path.
bars_path=/Path to BARS file in the dumped game/Base.bars

# This is the Original BWAV folder path.
stream_dir=/Path to BWAV folder in the dumped game/

# This is the Modded BWAV folder path.
mod_stream_dir=/Path to your mod's BWAV folder/

# This is the Patched BARS output path.
output_bars_path=/Path to your mod's BARS resource folder/Base.bars
```

The default configuration can be changed in the [default_config.h](/switch/src/default_config.h) file. Feel free to open pull requests adding new game configurations.

## Compiling

This program was made using the devkitpro switch tools and libraries.

To compile, make sure you have devkitpro switch-dev installed and simply run the makefile in this directory.
