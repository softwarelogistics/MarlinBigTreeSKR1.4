#include "../inc/MarlinConfig.h"

#if ENABLED(PNP_VACUUM_BRIDGE)
  #include "pnp_vacuum_bridge.h"
  #include "pnp_vacuum_protocol.h"
  #include "../core/serial.h"

  namespace {
    PnpVacuumProtocol pressure_frames;
  }

  void PnpVacuumBridge::consume(const char c) {
    pressure_frames.ingest(c, millis());
  }

  void PnpVacuumBridge::report() {
    int32_t prs1, prs2;
    if (!pressure_frames.snapshot(millis(), prs1, prs2)) {
      SERIAL_ECHOLNPGM("error:vacuum-stale");
      return;
    }
    SERIAL_ECHOLNPGM("prs1:", (long)prs1, " prs2:", (long)prs2);
  }
#endif
