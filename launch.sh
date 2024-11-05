#!/bin/bash -e

usage() {
    echo "Usage: $0 [-s] [-a] [-h]"
    echo "  -d    Run on debug mode"
    echo "  -h    Show help"
    exit 1
}

# Parse options with getopts
build_type="Release"
while getopts ":dh" option; do
    case "${option}" in
        d)
            echo "Run on debug mode"
            build_type="Debug"
            ;;
        h)
            usage
            ;;
        *)
            echo "Invalid option: -$OPTARG" >&2
            usage
            ;;
    esac
done

# Build
mkdir -p bin
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=$build_type ..
make
cd ..

# Kill previous process
pkill -f "IVRServer"
pkill -f "MediaServer"
pkill -f "SipServer"

# Get local IP
local_ip=$(hostname -I | awk '{print $1}')

# Set media directory to environment variable
export MEDIA_DIR=$(pwd)/blob
echo "Media directory: $MEDIA_DIR"

cd bin
echo "Run SIP Server on $local_ip:5060"
./SipServer --ip=$local_ip

echo "Run IVR Application on $local_ip"
./IVRServer -i $local_ip -p 5060 -c $local_ip -m 10000

echo "Run Media Server on $local_ip:9999, RTP port: 10000"
./MediaServer

