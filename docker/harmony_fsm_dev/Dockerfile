FROM ros:melodic

ENV DEBIAN_FRONTEND=noninteractive

# install packages
RUN apt-get update && apt-get dist-upgrade -q -y && apt-get install -q -y \
    dirmngr \
    gnupg2 \
    lsb-release \
    sudo \
    wget \
    rsync \
    python-dev \
    cmake \
    build-essential \
    gdb \
    python-catkin-tools \
    && rm -rf /var/lib/apt/lists/*

# setup environment
ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

# create an additional user "dev" for this image
ARG CUSTOM_UID=1000
ARG CUSTOM_GID=1000
ARG CUSTOM_USERNAME=dev

ENV HOME /home/$CUSTOM_USERNAME
#RUN mkdir /home/$CUSTOM_USERNAME

RUN getent group $CUSTOM_GID || $(groupadd --gid $CUSTOM_GID $CUSTOM_USERNAME)
RUN getent passwd $CUSTOM_UID || $(useradd --uid $CUSTOM_UID --gid $CUSTOM_GID -m -d /home/$CUSTOM_USERNAME -s /bin/bash $CUSTOM_USERNAME && \
    echo "${CUSTOM_USERNAME} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$CUSTOM_USERNAME && \
    chmod 0440 /etc/sudoers.d/$CUSTOM_USERNAME)


# you need to mount the project workspace volume in the docker launch, or vscode devcontainer
ARG PRJ_WS_DIR=/workspace
ENV PRJ_WS_DIR=${PRJ_WS_DIR}

RUN echo "source /opt/ros/melodic/setup.bash" >> /home/${CUSTOM_USERNAME}/.bash_aliases
RUN echo "source ${PRJ_WS_DIR}/devel/setup.bash" >> /home/${CUSTOM_USERNAME}/.bash_aliases
RUN echo "export ROSCONSOLE_FORMAT='[\${severity}] [\${time}] \${file}:\${line} \${message}'" >> /home/${CUSTOM_USERNAME}/.bash_aliases
RUN echo "[MASTER]" > /home/${CUSTOM_USERNAME}/.pylintrc
RUN echo "init-hook='import sys; sys.path.append(\"/opt/ros/melodic/lib/python2.7/dist-packages\")'" >> /home/${CUSTOM_USERNAME}/.pylintrc

WORKDIR ${PRJ_WS_DIR}
ENV USER ${CUSTOM_USERNAME}
USER ${CUSTOM_USERNAME}

# setup entrypoint
COPY ./entrypoint.sh /

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
