#!/bin/bash

# Copyright 2025 tobyzxj
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     https://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script is used to create a manifest file for the docker image.
# The manifest file is used to specify the architecture of the docker image.

# usage:
# ./manifest_create.sh [IMAGE]

IMAGE=$1
ISPUSH=$2
LINUX_ARCH="amd64 arm64"

if [ -z "$IMAGE" ]; then
    echo "Please specify the image name"
    exit 1
fi

# check if the docker manifest command is installed
if ! command -v docker > /dev/null; then
    echo "Please install docker"
    exit 1
fi

# check if the images exist, if not, pull them
# the images are pulled to check if they exist
for arch in $LINUX_ARCH; do
    if ! docker image inspect $IMAGE-$arch > /dev/null; then
        docker pull $IMAGE-$arch
        if [ $? -ne 0 ]; then
            echo "Failed to pull the image $IMAGE-$arch"
            exit 1
        fi
    fi
done

# create the manifest file
echo "Creating the manifest file for the image $IMAGE"
docker manifest create $IMAGE $IMAGE-amd64 $IMAGE-arm64
if [ $? -ne 0 ]; then
    echo "Failed to create the manifest file"
    exit 1
fi

# annotate the manifest file
echo "Annotating the manifest file for the image $IMAGE"
for arch in $LINUX_ARCH; do
    docker manifest annotate $IMAGE $IMAGE-$arch --os linux --arch $arch
    if [ $? -ne 0 ]; then
        echo "Failed to annotate the manifest file"
        exit 1
    fi
done

# push the manifest file
if [ "$ISPUSH" = "push" ]; then
    echo "Pushing the manifest file for the image $IMAGE"
    docker manifest push $IMAGE
fi
