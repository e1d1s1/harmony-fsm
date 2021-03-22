#!/bin/bash

docker build -t e1d1s1/harmony_fsm_dev --build-arg CUSTOM_UID=$UID --build-arg CUSTOM_GID=$GID ./harmony_fsm_dev
