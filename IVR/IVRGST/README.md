# IVR Application (Bind to a customized media server)

A simple IVR application

# Build

#### Linux 

```bash
  mkdir build && cd build
  cmake ..
  make
```
# Usage Guide
```bash
  cd build
  ./IVRServer -i 192.168.0.3 -p 5060 -c 192.168.0.3 -m 10000
```
