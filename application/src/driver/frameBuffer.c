#include "frameBuffer.h"


#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

#include <bcm_host.h>


int getFrame(void* outFrameBuff){
    DISPMANX_DISPLAY_HANDLE_T display;
    DISPMANX_MODEINFO_T display_info;
    DISPMANX_RESOURCE_HANDLE_T screen_resource;
    VC_IMAGE_TRANSFORM_T transform;
    uint32_t image_prt;
    VC_RECT_T rect1;
    int ret;
    int fbfd = 0;
    //char *fbp = 0;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;


    bcm_host_init();

    display = vc_dispmanx_display_open(0);
    if (!display) {
        syslog(LOG_ERR, "Unable to open primary display");
        return -1;
    }
    ret = vc_dispmanx_display_get_info(display, &display_info);
    if (ret) {
        syslog(LOG_ERR, "Unable to get primary display information");
        return -1;
    }
    syslog(LOG_INFO, "Primary display is %d x %d", display_info.width, display_info.height);


    fbfd = open("/dev/fb1", O_RDWR);
    if (fbfd == -1) {
        syslog(LOG_ERR, "Unable to open secondary display");
        return -1;
    }
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        syslog(LOG_ERR, "Unable to get secondary display information");
        return -1;
    }
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        syslog(LOG_ERR, "Unable to get secondary display information");
        return -1;
    }

    syslog(LOG_INFO, "Second display is %d x %d %dbps\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    //screen_resource = vc_dispmanx_resource_create(VC_IMAGE_RGB565, vinfo.xres, vinfo.yres, &image_prt);
screen_resource = vc_dispmanx_resource_create(VC_IMAGE_RGB565, 320, 240, &image_prt);
    if (!screen_resource) {
        syslog(LOG_ERR, "Unable to create screen buffer");
        close(fbfd);
        vc_dispmanx_display_close(display);
        return -1;
    }



///////fbp = (char*) mmap(0, 320*240*2, PROT_READ | PROT_WRITE, MAP_SHARED, fout, 0);
unsigned char fbp[320*240*2];
memset(fbp, 0, sizeof(fbp));

//    fbp = (char*) mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fbp <= 0) {
        syslog(LOG_ERR, "Unable to create mamory mapping");
        close(fbfd);
        ret = vc_dispmanx_resource_delete(screen_resource);
        vc_dispmanx_display_close(display);
        return -1;
    }

//    vc_dispmanx_rect_set(&rect1, 0, 0, vinfo.xres, vinfo.yres);

vc_dispmanx_rect_set(&rect1, 0, 0, 320, 240);
    while (1) {
        ret = vc_dispmanx_snapshot(display, screen_resource, 0);
	printf("------\n"); 
       vc_dispmanx_resource_read_data(screen_resource, &rect1, /*fbp*/outFrameBuff, 320 * 16 / 8);//vinfo.xres * vinfo.bits_per_pixel / 8);
	printf("do\n");        
usleep(25 * 1000);
	//write(fout, fbp, sizeof(fbp));
	break;
    }

    munmap(fbp, finfo.smem_len);
    close(fbfd);
    ret = vc_dispmanx_resource_delete(screen_resource);
    vc_dispmanx_display_close(display);
    return 0;
}
