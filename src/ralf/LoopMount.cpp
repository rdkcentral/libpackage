#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <linux/loop.h>

namespace packagemanager
{
    // Helper: attach a file to a free loop device
    int setup_loop_device(const char *img_path, char *loop_dev, size_t loop_dev_size)
    {
        int ctl_fd = -1, img_fd = -1, loop_fd = -1;
        int loop_num;

        // Open loop-control to get a free loop device number
        ctl_fd = open("/dev/loop-control", O_RDWR);
        if (ctl_fd < 0)
        {
            perror("open /dev/loop-control");
            return -1;
        }

        loop_num = ioctl(ctl_fd, LOOP_CTL_GET_FREE);
        close(ctl_fd);
        if (loop_num < 0)
        {
            perror("LOOP_CTL_GET_FREE");
            return -1;
        }

        snprintf(loop_dev, loop_dev_size, "/dev/loop%d", loop_num);

        // Open the image file
        img_fd = open(img_path, O_RDONLY);
        if (img_fd < 0)
        {
            perror("open image");
            return -1;
        }

        // Open the loop device
        loop_fd = open(loop_dev, O_RDWR);
        if (loop_fd < 0)
        {
            perror("open loop device");
            close(img_fd);
            return -1;
        }

        // Associate the image with the loop device
        if (ioctl(loop_fd, LOOP_SET_FD, img_fd) < 0)
        {
            perror("LOOP_SET_FD");
            close(img_fd);
            close(loop_fd);
            return -1;
        }

        // Configure loop device info
        struct loop_info64 loopinfo;
        memset(&loopinfo, 0, sizeof(loopinfo));
        strncpy((char *)loopinfo.lo_file_name, img_path, LO_NAME_SIZE - 1);
        loopinfo.lo_flags = LO_FLAGS_READ_ONLY;

        if (ioctl(loop_fd, LOOP_SET_STATUS64, &loopinfo) < 0)
        {
            perror("LOOP_SET_STATUS64");
            ioctl(loop_fd, LOOP_CLR_FD, 0);
            close(img_fd);
            close(loop_fd);
            return -1;
        }

        close(img_fd);
        close(loop_fd);
        return 0;
    }

    // Helper: detach loop device
    void detach_loop_device(const char *loop_dev)
    {
        int fd = open(loop_dev, O_RDWR);
        if (fd >= 0)
        {
            ioctl(fd, LOOP_CLR_FD, 0);
            close(fd);
        }
    }
} // namespace Packagepackagemanager