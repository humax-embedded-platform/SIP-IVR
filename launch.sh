#!/bin/bash -e

build_type="Release"

usage() {
    echo "Usage: $0 [-d] [-h]"
    echo "  -d    Run on debug mode"
    echo "  -h    Show help"
    exit 0
}

run() {
    # Build
    mkdir -p bin
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=$build_type ..
    make
    cd ..

    # Kill previous process
    echo "Kill previous process"
    if pgrep -x "IVRServer" > /dev/null
    then
        pkill -9 IVRServer
    fi

    if pgrep -x "MediaServer" > /dev/null
    then
        pkill -9 MediaServer
    fi

    if pgrep -x "SipServer" > /dev/null
    then
        pkill -9 SipServer
    fi

    # Get local IP
    local_ip=$(hostname -I | awk '{print $1}')
    echo "Local IP: $local_ip"

    # Set media directory to environment variable
    export MEDIA_DIR=$(pwd)/blob

    cd bin
    echo "Run SIP Server on $local_ip:5060"
    gnome-terminal -- bash -c "./SipServer --ip=$local_ip"

    echo "Run IVR Application on $local_ip"
    ## open terminal with set name for terminal
    
    gnome-terminal -- bash -c "./IVRServer -i $local_ip -p 5060 -c $local_ip -m 10000"

    echo "Run Media Server on $local_ip:9999, RTP port: 10000"
    gnome-terminal -- bash -c "./MediaServer"
}

# Parse options with getopts
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

# run
