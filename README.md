# ZuPi - Raspberry Pi on Zumo Robot

**WORK IN PROGRESS**

This repository contains the source code for ZuPi projects - the robots built
with Raspberry Pi (or compatible alternatives) on
[Zumo](https://www.pololu.com/category/129/zumo-robots-and-accessories) robot.

_NOTE: a lot of in-progress/early-stage work, lack of docs, tests, likely to break!_

Looking for documents about the details, including hardware and software? Please
visit the gitbook about this project [ZuPi Build Memo](https://easeway.gitbooks.io/zupi-build-memo/content/).

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

### Deploy And Play

Install the dependencies:

- [Node.js](http://nodejs.org) version 4.5 or above
- [v4l2rtspserver](https://github.com/mpromonet/v4l2rtspserver) in `PATH`
- Enable camera and I2C (for servo controller)

Deploy robot package

```
hmake pack-zpi1-robot
scp out/zpi1-robot-0.0.1.tgz pi@robot:/root/
ssh pi@robot
    tar zxf zpi1-robot-0.0.1.tgz
    cd package
    npm install  # this takes a while
    DEBUG=* node app.js
```

Now the robot is looking for _brain_ (the controlling logic)...

On the controller (laptop or a server), build the brain

```
hmake build-zpi1-brain-amd64 build-zpi1-vision-amd64
export PATH=$PATH:`pwd`/out/amd64/bin
LOGXI=*=DBG zpi1-brain brain
```

When the brain is connected, it will start controlling the robot...
and enjoy!

### Simulation

Folder `sim/smartcam1d` contains the simulation code to experiment the controlling
logic of moving the camera. It's easy to add/modify algorithms (see `algo_*.*` files).

To build the code

```
hmake build-smartcam1d-amd64
```

You need `sim-ng` (see [Simulator](https://github.com/robotalks/simulator)) installed
in `PATH`:

```
sim-ng vis -I sim/visualizer -- out/amd64/smartcam1d sim -l fido
```

Point your browser to `http://localhost:3500`, and you will see the object and camera.
The reinforcement learning algorithm tries to move the camera to center the object.
Once it centered the object, click on the screen to replace the object and let
the algorithm center the object again.
And you will see how fast the algorithm converges, or maybe never...

## References

The project is based on technologies of

- [ThingsBus](https://github.com/robotalks/tbus)
- [Simulator](https://github.com/robotalks/simulator)
