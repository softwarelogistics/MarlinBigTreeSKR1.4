#include "../Marlin/src/gcode/pnp_vacuum_protocol.h"
#include <cassert>
#include <cstdint>

static void send(PnpVacuumProtocol &parser, const char *message, uint32_t time) {
  for (const char *p = message; *p; ++p) parser.ingest(*p, time);
}

int main() {
  PnpVacuumProtocol p;
  int32_t a = 0, b = 0;
  assert(!p.snapshot(0, a, b));

  send(p, "prs1:-38000 prs2:12345\r\n", 100);
  assert(p.snapshot(101, a, b) && a == -38000 && b == 12345);
  send(p, "prs1:111 prs2:-222\n", 200);
  assert(p.snapshot(950, a, b) && a == 111 && b == -222);
  assert(!p.snapshot(951, a, b));
  send(p, "prs1:8388607 prs2:-8388608\n", 1000);
  assert(p.snapshot(1000, a, b) && a == 8388607 && b == -8388608);
  send(p, "prs1:8388608 prs2:2\n", 1010);
  assert(p.snapshot(1011, a, b) && a == 8388607 && b == -8388608);
  send(p, "garbage\n", 1020);
  assert(p.snapshot(1021, a, b) && a == 8388607 && b == -8388608);
  send(p, "prs1:32 prs2:3\n", 0xFFFFFFF0u);
  assert(p.snapshot(5u, a, b) && a == 32 && b == 3);
  send(p, "prs1:234 prs2:567xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n", 7);
  assert(p.snapshot(8u, a, b) && a == 32 && b == 3);
}
