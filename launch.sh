#!/bin/bash -e

build_type="Release"
media_dir=""

usage() {
    echo "Usage: $0 [-d] [-h]"
    echo "  -d    Run on debug mode"
    echo "  -m    Media blob directory path"
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
    if pgrep -x "sip-ivr" > /dev/null
    then
        pkill -9 sip-ivr
    fi

    # Get local IP
    local_ip=$(hostname -I | awk '{print $1}')
    echo "Local IP: $local_ip"

    # Set media directory to environment variable
    export MEDIA_DIR=$(pwd)/blob

    cd bin
    echo "Run SIP Server on $local_ip:5060"

    ./sip-ivr -i $local_ip -m /home/phongdang/WorkSpace/SipServer/blob
}

# Parse options with getopts
while getopts ":dmh" option; do
    case "${option}" in
        d)
            echo "Run on debug mode"
            build_type="Debug"
            ;;
        m)  
            media_dir=${OPTARG}
            echo "Media directory: $media_dir"
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

run
