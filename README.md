# ZuPi - Raspberry Pi on Zumo Robot

This repository contains the source code for ZuPi projects - the robots built
with Raspberry Pi (or compatible alternatives) on
[Zumo](https://www.pololu.com/category/129/zumo-robots-and-accessories) robot.

_NOTE: a lot of in-progress/early-stage work, lack of docs, tests, likely to break!_

## Getting Started

Linux is the only supported platform, however you can build from Mac/Windows,
thanks to [HyperMake](https://evo-cloud.github.io/hmake).


To build everything:

```
hmake build-*
```

To build individual targets:

```
hmake build-zpi1-brain-*   # build zpi1 brain for multiple archs
hmake build-zpi1-vision-*  # build zpi1 compute vision module for multiple archs
hmake pack-zpi1-robot      # create Node.js package for robot logic (tbus based)
hmake build-smartcam1d-*   # build smartcam1d algorithm simulation
```

## References

The project is based on technologies of

- [ThingsBus](https://github.com/robotalks/tbus)
- [Simulator](https://github.com/robotalks/simulator)
