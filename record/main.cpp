#include <iostream>
#include <signal.h>

#include "ps4eye.h"

// Example code taken from
//  http://blog.matthias.roehser.de/?p=97
//  by Matthias Röhser

using namespace std;

static ps4eye *ps4cam = NULL;
int abort_count = 0;

static void signal_handler(int signum)
{
  switch (signum) {
  case SIGINT:
    cout << endl << "SIGINT!" << endl;
    abort_count++;
    if (ps4cam) {
      ps4cam->stop();
      if (abort_count>=5)
        delete ps4cam;
    }
    if (abort_count>=10)
      exit(-1);
    break;
  }
}

int main(void)
{
  ps4cam = new ps4eye;

#if !defined(_MBCS) // ignore signals in windows
  struct sigaction sigact;
  sigact.sa_handler = signal_handler;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = 0;
  sigaction(SIGINT, &sigact, NULL);
#endif

  // setup usb configuration
  ps4cam->init();

  // Start transferring data. This function never returns
  ps4cam->play();
  cout << "Exiting..." << endl;
  delete ps4cam;
  return 0;
}
