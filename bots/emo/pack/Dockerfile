FROM container4armhf/armhf-busybox
ARG ARCH=armhf
ADD bin/emo-bot /bin/
ADD bin/mosquitto /bin/
ADD bin/daemon /bin/daemon
ADD bin/minit /sbin/
ADD assets/emo/minit /etc/minit
ENTRYPOINT ["/sbin/minit"]
