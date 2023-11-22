# file.driver
A [CyberSound](https://aminet.net/package/disk/cdrom/14Bit_CDPlayer) sound driver which writes the audio stream as 16-bit PCM to a file.

Created to be able to redirect output of [GMPlay](https://aminet.net/package/mus/midi/GMPlay13) to PIPE:.


## Installation

Copy file.driver to DEVS:SoundDrivers/ or GMPlay/SoundDrivers/.


## Examples

```
> GMPlay MIDI-Files/Swamp_Thing.MID OUTPUT=file FREQUENCY=22050
> Play16 T:tune.pcm RAW BITS=16 TRACKS=2 FREQ=22050
```

```
> version FULL AUDIO:
AHI-Handler 4.2 (1997-04-09)
> SetEnv CyberSound/SoundDrivers/file_Destination AUDIO:TYPE/SIGNED/BITS/16/CHANNELS/2/FREQUENCY/22050
> GMPlay MIDI-Files/Swamp_Thing.MID OUTPUT=file FREQUENCY=22050
```

```
> SetEnv CyberSound/SoundDrivers/file_Destination PIPE:stream.pcm 
> Run GMPlay MIDI-Files/Swamp_Thing.MID OUTPUT=file FREQUENCY=22050
> <insert PIPE:-compatible cmdline player> PIPE:stream.pcm FREQUENCY=22050
```
