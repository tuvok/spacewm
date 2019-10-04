# SpaceWM

SpaceWM is a simple compositing Window Manager initially based on basic_wm:
https://github.com/jichu4n/basic_wm

## Getting Started

Currently there is not much to see here, but if you want to try this out, see the procedure below.

### Prerequisites

Requirements:
X11 server with Compositing + development libraries


### Installing

```
git clone -> ${spacewm}
cd ${spacewm}
mkdir build
cd build
cmake ../
make
cp ../xinitrc ./
xinit ./xinitrc
```
