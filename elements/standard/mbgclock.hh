#ifndef CLICK_MBGCLOCK_HH
#define CLICK_MBGCLOCK_HH
#include <click/element.hh>
#include <click/timer.hh>
#include <click/task.hh>
#include <click/sync.hh>
#undef HAVE_CONFIG_H //Clash with Meinberg's same define
#include <mbgdevio.h>
#include <mbgpccyc.h>

CLICK_DECLS

/* =c
 * MBGClock()
 * =s
 * Use meinberg device to get time
 * =d
 *
 * Click needs to be compiled with --enable-user-timestamp for this to be used.
 *
 */

class MBGClock : public Element { public:

  MBGClock() CLICK_COLD;

  const char *class_name() const        { return "MBGClock"; }
  const char *port_count() const        { return PORTS_0_0; }

  int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
  int initialize(ErrorHandler *) CLICK_COLD;
  void cleanup(CleanupStage);

  uint64_t now();
  uint64_t cycles();

  enum {h_now, h_cycles, h_now_steady};
  static String read_handler(Element *e, void *thunk);
  void add_handlers() CLICK_COLD;

private:
  int _verbose;
  bool _install;
  MBG_DEV_HANDLE _dh;
};


CLICK_ENDDECLS
#endif
