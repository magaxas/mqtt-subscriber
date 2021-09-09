#!/bin/bash

source ./env.sh

if [[ "$MENUCONFIG" == "true" ]]; then
    cd $RUTOS_DIR
    make menuconfig
    cd -
fi

for K in "${!PACKAGES[@]}"
do
    if [[ "${PACKAGES[$K]}" == "true" ]]; then
        cp -r $K $RUTOS_DIR/package
        echo "Copied $K to $RUTOS_DIR/packages"
        cd $RUTOS_DIR
        echo "Building $K..."
        make -j$(($(nproc)+1)) package/$K/compile
        cd -
    else
        echo "Skipping $K"
    fi
done


if [[ "$SCP" == "true" ]]; then
    IPKS=()
    cd $IPK_DIR
    for K in "${!PACKAGES[@]}"
    do
        IPKS+=$(echo "$K"*)" "
    done

    echo "IPKS to upload:"
    echo "$IPKS"
    scp $IPKS $USER@$IP:$SCP_DIR
fi
