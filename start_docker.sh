#!/bin/bash
DOCKER_BASE_NAME=e1d1s1/harmony_fsm_dev
CONTAINER_NAME=harmony_fsm_dev
SRC_DIR=$PWD

OPTIONS=" -v $SRC_DIR:/workspace:rw \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  --env=COLUMNS=`tput cols` --env=LINES=`tput lines` \
  --env=DISPLAY --env=QT_X11_NO_MITSHM=1 -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v $HOME/.ssh:/home/dev/.ssh:ro \
  -v $HOME/:/home/dev/hosthome:rw \
  -v $SRC_DIR/.docker_bash_history:/home/dev/.bash_history:rw \
  -v $SRC_DIR/.docker_git_config:/home/dev/.gitconfig:rw "

echo "Attach more bash shells with:"
echo "   docker exec -it $DOCKER_BASE_NAME bash"
echo "You can also attach the vscode IDE to the running container"

# persist things in the docker using the source directory
if [[ ! -f $SRC_DIR/.docker_bash_history ]]; then
  touch $SRC_DIR/.docker_bash_history
fi

if [[ ! -f $SRC_DIR/.docker_git_config ]]; then
  if [[ -f $HOME/.gitconfig ]]; then
    cp "$HOME/.gitconfig" "$SRC_DIR/.docker_git_config"
  fi
fi

docker run -it --gpus=all --privileged --log-driver=syslog --net=host `echo $OPTIONS` -u dev --rm --name $CONTAINER_NAME $DOCKER_BASE_NAME

