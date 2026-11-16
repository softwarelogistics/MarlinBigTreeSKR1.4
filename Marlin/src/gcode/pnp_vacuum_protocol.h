#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Parser is independent of Marlin internals so it can be tested with native C++.
class PnpVacuumProtocol {
public:
  void ingest(const char ch, const uint32_t received_ms) {
    if (ch == '\r' || ch == '\n') {
      if (count_ && !discard_) {
        line_[count_] = '\0';
        parse(received_ms);
      }
      count_ = 0;
      discard_ = false;
    }
    else if (!discard_) {
      if (count_ < sizeof(line_) - 1)
        line_[count_++] = ch;
      else {
        count_ = 0;
        discard_ = true;
      }
    }
  }

  bool snapshot(const uint32_t now_ms, int32_t &prs1, int32_t &prs2) const {
    if (!valid_ || uint32_t(now_ms - received_ms_) > 750UL) return false;
    prs1 = prs1_;
    prs2 = prs2_;
    return true;
  }

private:
  void parse(const uint32_t received_ms) {
    if (strncmp(line_, "prs1:", 5) != 0) return;
    char *end;
    const char *const first = line_ + 5;
    const long value1 = strtol(first, &end, 10);
    if (end == first || strncmp(end, " prs2:", 6) != 0) return;

    const char *const second = end + 6;
    const long value2 = strtol(second, &end, 10);
    if (end == second || *end != '\0') return;
    if (value1 < -8388608L || value1 > 8388607L
      || value2 < -8388608L || value2 > 8388607L) return;

    prs1_ = int32_t(value1);
    prs2_ = int32_t(value2);
    received_ms_ = received_ms;
    valid_ = true;
  }

  char line_[48] = {};
  uint8_t count_ = 0;
  bool discard_ = false, valid_ = false;
  int32_t prs1_ = 0, prs2_ = 0;
  uint32_t received_ms_ = 0;
};
