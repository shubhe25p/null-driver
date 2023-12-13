sudo insmod samplenulldev.ko
sudo mknod /dev/samplenulldev c 420 0
sudo chmod 666 /dev/samplenulldev