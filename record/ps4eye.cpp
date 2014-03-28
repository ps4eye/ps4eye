#include "ps4eye.h"

#include <iomanip>
#include <unistd.h>
#define debug(x...)  fprintf(stdout,x)
using namespace std;

/**
 * Initialize variables and USB communication.
 * Initialize device.
 */
ps4eye::ps4eye() {
  dev = NULL;
  context = NULL;
  handle = NULL;
  returned = false;
  abort = false;

  // hardcode samplerate. The device supports 44100, 48000
  samplerate = 48000;

  // just take something bigger, that fits
  buffersize = 5571968;

  // create a usb control trasfer packet. This is used to send commands to the device.
  control_transfer = libusb_alloc_transfer(0);
  control_transfer_buffer = new unsigned char[72];

  // big buffers to store data
  video_in_buffer = new unsigned char[buffersize+20];
}

/**
 * Free the usb handles and destroy all data structures.
 */
ps4eye::~ps4eye() {
  abort = true;
  usleep(1000000);
  release_usb();
  libusb_free_transfer(control_transfer);
  delete[] control_transfer_buffer;
  delete[] video_in_buffer;
}

void ps4eye::init() {
  // check if firmware needs to be uploaded
  firmware_upload();
  // initialize usb communication
  setup_usb();
  cout << "Initializing PS4 camera..." << endl;
  startup_commands();
}

void ps4eye::stop() {
  abort = true;
}

void ps4eye::firmware_upload() {

  int error;
  error = libusb_init(&context);
  if (error < 0) {
    exit(error);
  }
  libusb_set_debug(context, 3);

  handle = libusb_open_device_with_vid_pid(context, 0x05a9, 0x0580);
  if (handle == NULL) {
    return;
  }
  dev = libusb_get_device(handle);

  libusb_reset_device(handle);
  libusb_set_configuration(handle, 0);
  libusb_ref_device(dev);
  libusb_claim_interface(handle, 0);

  cout << "Uploading firmware to PS4 camera..." << endl;

  uint16_t chunk_size = 512;

  uchar chunk[chunk_size+8];

  ifstream firmware("firmware.bin", ios::in|ios::binary|ios::ate);
  if (firmware.is_open())
  {
    uint32_t length = firmware.tellg();
    firmware.seekg(0, ios::beg);

    uint16_t index=0x14;
    uint16_t value=0;

    for (uint32_t pos=0; pos<length; pos+=chunk_size) {
      uint16_t size = ( chunk_size > (length-pos) ? (length-pos) : chunk_size);
      firmware.read((char*)(chunk+8), size);
      submitAndWait_controlTransfer(0x40, 0x0, value, index, size, chunk);
      if ( ((uint32_t)value + size) > 0xFFFF ) index+=1;
      value+=size;
    }
    firmware.close();

    chunk[8] = 0x5b;
    submit_controlTransfer(0x40, 0x0, 0x2200, 0x8018, 1, chunk);
    usleep(1000000);
    libusb_cancel_transfer(control_transfer);

    cout << "Firmware transferred and device reset, run again to record." << endl;
  } else {
    cout << "Unable to open firmware.bin!" << endl;
  }

  libusb_release_interface(handle, 0);
  libusb_unref_device(dev);
  libusb_close(handle);
  libusb_exit(context);

  exit(0);
}

/**
 * Setup usb communication and request access to the device from the system.
 */
void ps4eye::setup_usb() {

  int error;
  error = libusb_init(&context);
  if (error < 0) {
    exit(error);
  }
  libusb_set_debug(context, 3);

  handle = libusb_open_device_with_vid_pid(context, 0x05a9, 0x058a);
  if (handle == NULL) {
    cout << "Failed to get device handle of Ps4cam" << endl;
    exit(1);
  }
  dev = libusb_get_device(handle);

  error = libusb_kernel_driver_active(handle, 1);
  if (error) {
    cout << "WARNING: Kernel driver active for interface 1: " << (int) error << endl;
    /*
    libusb_detach_kernel_driver(handle, 1);
    if (libusb_kernel_driver_active(handle, 1))
      exit(1);
    */
  }
  // libusb_set_debug(context, 5);

  usleep(2000);

  // reset usb communication
  libusb_reset_device(handle);
  libusb_set_configuration(handle, 1);
  libusb_ref_device(dev);
  //libusb_claim_interface(handle, 0);
  libusb_claim_interface(handle, 1);
  //libusb_set_interface_alt_setting(handle, 0, 1);
  libusb_set_interface_alt_setting(handle, 1, 1);

  usleep(1000);
}

/**
 * Release all usb data
 */
void ps4eye::release_usb() {
  libusb_release_interface(handle, 0);
  libusb_release_interface(handle, 1);
  libusb_unref_device(dev);
  libusb_close(handle);
  libusb_exit(context);
}

/**
 * Initialize device. This is also called, when the device lost synchronization due to
 * some system timeout.
 *
 * This function replays all events, that Wireshark logs when the original Ps4cam
 * driver connects to the device.
 * What these commands mean is mostly unknown to me.

 */
void ps4eye::startup_commands() {
  ifstream commands("startup.bin", ios::in | ios::binary);
  uint data_size = 64;
  uchar *data = (uchar*)malloc(data_size+8);
  uchar *dev_data = (uchar*)malloc(data_size+8);
  uchar setup[8];
  int cmd_index = 0;

  if (!commands.is_open()) {
    cout << "unable to open commands file" << endl;
    abort = true;
  }

  // command #667 @ 0xbb38 turns led on
  while (!commands.eof() && !abort) {
    cmd_index++;

    int pos = commands.tellg();
    commands.read((char*)setup,8);

    uint8_t bmRequestType = setup[0];
    uint8_t bRequest      = setup[1];
    uint16_t wValue       = setup[2] + setup[3]*0x100;
    uint16_t wIndex       = setup[4] + setup[5]*0x100;
    uint16_t wLength      = setup[6] + setup[7]*0x100;

    cout << setfill('0') << dec << cmd_index << " - "
         << setw(4) << hex << (int)pos << " - "
         << setw(2) << hex << (int)bmRequestType << ":"
         << setw(2) << hex << (int)bRequest << ":"
         << setw(4) << hex << (int)wValue << ":"
         << setw(4) << hex << (int)wIndex << ":"
         << setw(4) << hex << (int)wLength << endl;

    if (wLength > data_size) {
      cout << "resizing data buffer to wLength " << wLength << endl;
      data = (uchar*)realloc(data,wLength+8);
      dev_data = (uchar*)realloc(data,wLength+8);
      data_size = wLength;
    }
    commands.read((char*)(data+8),wLength);

    bool success = false;
    usleep(3200);
    submitAndWait_controlTransfer(bmRequestType, bRequest, wValue, wIndex, wLength,
                                  (bmRequestType & LIBUSB_ENDPOINT_IN ? dev_data : data));

    success = true;
    if (bmRequestType & LIBUSB_ENDPOINT_IN) {
      for (int i=0; i<wLength; i++) {
        if (dev_data[8+i] != data[8+i]) {
          cout << dec << " diff @ " << i << " : " << setfill('0')
               << setw(2) << hex << (uint)dev_data[8+i] << " / " << setw(2) << hex << (uint)data[8+i] << endl;
        }
      }
    }
  }

}

/**
 * When the usb transfers timeout or the device doesn't get video packets fast enough,
 * it has to be resumed.
 */
void ps4eye::resumePlayback() {
  cerr << "playback resumed" << endl << flush;
  submit_controlTransfer(0x40, 0x49, 0x0032, 0, 0, control_transfer_buffer);
}

/**
 * Submit a control transfer to the usb system and wait until this packet has
 * been processed.
 */
void ps4eye::submitAndWait_controlTransfer(uint8_t bmRequestType,
                                           uint8_t bRequest,
                                           uint16_t wValue,
                                           uint16_t wIndex,
                                           uint16_t wLength,
                                           uchar *buffer ) {
  submit_controlTransfer(bmRequestType, bRequest, wValue, wIndex, wLength, buffer);
  controlTransferReturned = false;
  while (!controlTransferReturned)
    libusb_handle_events(context);
}

/**
 * Submit a control transfer to the usb system and continue with the program flow, i.e.
 * do not wait until the packet has been processed.
 */
void ps4eye::submit_controlTransfer(uint8_t bmRequestType,
                                    uint8_t bRequest,
                                    uint16_t wValue,
                                    uint16_t wIndex,
                                    uint16_t wLength,
                                    uchar *buffer ) {
  libusb_fill_control_setup(buffer, bmRequestType, bRequest,
                            wValue, wIndex, wLength);
  libusb_fill_control_transfer(control_transfer, handle, buffer,
                               (libusb_transfer_cb_fn)callback_controlTransfer, NULL, 1000);
  control_transfer->user_data = this;
  control_wLength = wLength;
  libusb_submit_transfer(control_transfer);
}

/**
 * This method is called when a control transfer has been completed.
 */
void ps4eye::callback_controlTransfer(struct libusb_transfer * transfer) {
  ps4eye * ps4cam = (ps4eye*) transfer->user_data;
  ps4cam->controlTransferReturned = true;
  if (ps4cam->control_wLength != transfer->actual_length) {
    cout << "ERROR: " << transfer->actual_length << " / " << ps4cam->control_wLength << " bytes transferred" << endl;
    ps4cam->stop();
  }
}

/**
 * Prepare a data packet, that sends video to the host using an isochronous transfer.
 */
struct libusb_transfer * ps4eye::allocate_iso_input_transfer() {
  struct libusb_transfer * transfer = libusb_alloc_transfer(136);

  if (!transfer)
    exit(1);

  // prepare the data packet and populate the neccessary fields
  //transfer->flags = LIBUSB_TRANSFER_ADD_ZERO_PACKET;
  transfer->dev_handle = this->handle;
  transfer->endpoint = 0x81;
  transfer->type = LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
  transfer->timeout = 100;
  transfer->buffer = video_in_buffer;
  memset(transfer->buffer, 0, buffersize);
  transfer->user_data = this;
  transfer->length = 5571968; //max size
  transfer->callback = (libusb_transfer_cb_fn)ps4eye::callback_videoin;
  transfer->num_iso_packets = 136; //num isoc

  for (unsigned int i = 0; i < transfer->num_iso_packets; i++) {
    transfer->iso_packet_desc[i].length = 40*1024;
  }

  return transfer;
}
/**
 * This method is called after a video packet has been transferred from the device.
 * Avoid printf inside callback, try to call this instead
 */
void ps4eye::debug_callback(struct libusb_transfer * transfer)
{

  int len=1024*40;
  int length=0;
  unsigned char* data=transfer->buffer;
  //debug("begin debug callback\n");
  for(int i=0; i<transfer->num_iso_packets; i++)
  {
    if(transfer->iso_packet_desc[i].actual_length!=0)
    {
      debug("Package: %d \n",i);
      debug("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x len %d\n",data[0+len*i],data[1+len*i],data[2+len*i],data[3+len*i],data[4+len*i],data[5+len*i],data[6+len*i],data[7+len*i],data[8+len*i],data[9+len*i],data[10+len*i],data[11+len*i],transfer->iso_packet_desc[i].actual_length);
    }
    length=length+transfer->iso_packet_desc[i].actual_length;

  }
  if (length!=0)
    debug("end debug callback bytes received %d\n",length);

}
/**
 * This method is called after a video packet has been transferred from the device.
 * If the transfer fails, the device is reset. Request the next data packet afterwards.
 */
void ps4eye::callback_videoin(struct libusb_transfer * transfer) {

  ps4eye * ps4cam = (ps4eye*) transfer->user_data;
  unsigned char * data=transfer->buffer;
  // if the usb connection breaks down, resume it
  if (transfer->status != 0) {
    debug("bad status transfer: %d\n",transfer->status);
    //ps4cam->resumePlayback();
  }
  debug_callback(transfer);

  transfer->buffer = ps4cam->video_in_buffer;

  // Request next data packet.
  if (!ps4cam->abort)
    libusb_submit_transfer(transfer);
}

/**
 * "Main loop" of the device driver. It starts the transfers and goes into an
 * infinite loop that polls the usb system for events.
 */
void ps4eye::play() {
  if (abort)
    return;
  debug("begin debug playback\n");
  video_transfer = allocate_iso_input_transfer();
  if (libusb_submit_transfer(video_transfer) < 0)
    printf("Submitting play transfers failed.");

  while (!abort) {
    if (libusb_handle_events(context) < 0)
      break;
  }
}
