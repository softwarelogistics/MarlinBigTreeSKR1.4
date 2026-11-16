#pragma once
namespace PnpVacuumBridge {
  void consume(char c); // Marlin UART3 received byte
  void report();        // Host M170 request
}
