#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <libusb.h>

using namespace std;

#define DEFAULT_TIMEOUT    2000

typedef unsigned char uchar;

static const uint vid = 0x05a9;
static const uint pid = 0x058a;
static const char* command_file = "startup.bin";

#define MAX_PACKET 5677056
#define CHUNK_SIZE 1024

static uchar imgbuf[MAX_PACKET];
static int do_exit = 0;
static struct libusb_transfer *img_transfer = NULL;
static int img_idx = 0;

static int save_to_file(unsigned char *data, int size)
{
        FILE *fd;
        char filename[64];
        snprintf(filename, sizeof(filename), "dump%d.raw", img_idx++);
        fd = fopen(filename, "w");
        if (!fd)
                return -1;
        (void) fwrite(data, 1, size, fd);
        fclose(fd);
        printf("saved image to %s\n", filename);
        return 0;
}

static void LIBUSB_CALL cb_img(struct libusb_transfer *transfer)
{
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
                fprintf(stderr, "img transfer status %d?\n", transfer->status);
                //do_exit = 2;
                //libusb_free_transfer(transfer);
                //img_transfer = NULL;
                return;
        }
        cout << "Payload callback" << endl;
        save_to_file(imgbuf, transfer->actual_length);
        if (libusb_submit_transfer(img_transfer) < 0) {
          cout << "should do exit?" << endl;
          do_exit = 2;
        }
}

static void sig_hdlr(int signum)
{
  switch (signum) {
  case SIGINT:
    cout << "in sig handler" << endl;
    do_exit = 1;
    break;
  }
}

int main() {

  libusb_context *ctx = NULL;
  ssize_t cnt;

  if(libusb_init(&ctx) < 0) {
    cout << "libusb init error!" << endl;
    return -1;
  }
  libusb_set_debug(ctx, 3);

  libusb_device_handle *udevh = libusb_open_device_with_vid_pid(ctx, vid, pid);

  cout << "device " << setfill('0') << setw(4) << hex << vid << ":" << setw(4) << pid
       << (!udevh?" not":"") << " found" << endl;

  if (!udevh) {
    return -1;
  }

  if (libusb_kernel_driver_active(udevh, 1) == 1) {
    if (libusb_detach_kernel_driver(udevh, 1) == 0) {
      cout << "Kernel driver detached!" << endl;
    } else {
      cout << "Unable to detach kernel driver!" << endl;
      return -1;
    }
  }

  if (libusb_claim_interface(udevh, 1) < 0) {
    cout << "Cannot claim interface!" << endl;
    return -1;
  }

  if (libusb_set_interface_alt_setting(udevh,1,1) != 0) {
    cout << "Unable to set alternate setting!" << endl;
    return -1;
  }

  ifstream commands(command_file, ios::in | ios::binary);
  uint data_size = 64;
  uchar *data = (uchar*)malloc(data_size);
  uchar *dev_data = (uchar*)malloc(data_size);
  uchar setup[8];
  int cmd_index = 0;

  // command #667 @ 0xbb38 turns led on

  while (!commands.eof()) {
    cmd_index++;

    int pos = commands.tellg();
    commands.read((char*)setup,8);
    uint wtype     = setup[0];
    uint wrequest  = setup[1];
    uint wvalue    = setup[2] + setup[3]*0x100;
    uint windex    = setup[4] + setup[5]*0x100;
    uint dlength   = setup[6] + setup[7]*0x100;

    cout << dec << cmd_index << " - "
         << setw(4) << hex << pos << " - "
         << setw(2) << hex << wtype << ":"
         << setw(2) << hex << wrequest << ":"
         << setw(4) << hex << wvalue << ":"
         << setw(4) << hex << windex << ":"
         << setw(4) << hex << dlength << endl;

    if (dlength > data_size) {
      cout << "resizing data buffer to dlength " << dlength << endl;
      data = (uchar*)realloc(data,dlength);
      dev_data = (uchar*)realloc(data,dlength);
      data_size = dlength;
    }

    commands.read((char*)data,dlength);

    usleep(3200);
    int ret = libusb_control_transfer(udevh, wtype, wrequest, wvalue, windex,
                                      (wtype & LIBUSB_ENDPOINT_IN ? dev_data : data), dlength, DEFAULT_TIMEOUT);

    if (ret != dlength) {
      cerr << ret << "/" << dlength << " bytes transmitted" << endl;
      return -1;
    }

    if (wtype & LIBUSB_ENDPOINT_IN) {
      for (int i=0; i<dlength; i++) {
        if (dev_data[i] != data[i]) {
          cout << dec << " diff @ " << i << " : " << setfill('0')
               << setw(2) << hex << (uint)dev_data[i] << " / " << setw(2) << hex << (uint)data[i] << endl;
        }
      }
    }
  }

  cout << "teh commands were sent!" << endl;

  img_transfer = libusb_alloc_transfer(MAX_PACKET);
  if (!img_transfer)
    return -1;
  libusb_fill_iso_transfer(img_transfer, udevh, 0x81, imgbuf,
                           MAX_PACKET, CHUNK_SIZE, cb_img, NULL, 0);

  libusb_set_iso_packet_lengths(img_transfer, MAX_PACKET/CHUNK_SIZE);

  struct sigaction sigact;

  sigact.sa_handler = sig_hdlr;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = 0;
  sigaction(SIGINT, &sigact, NULL);

  if (libusb_submit_transfer(img_transfer) < 0) {
    cout << "Error submitting for transfer!" << endl;
    return -1;
  }

  while (!do_exit) {
    if (libusb_handle_events(NULL) != LIBUSB_SUCCESS)
      break;
  }

  libusb_close(udevh);
  libusb_exit(ctx);
  return 0;
}
