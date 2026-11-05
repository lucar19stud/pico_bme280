# Pico BME280 Test System
**Status:** UART output and capture working. BME280 driver and tests in progress.

I wrote this project to get practical experience with C on a microcontroller and with bus protocols
I had only seen in lectures. A Pico 2 W reads a BME280 sensor over I2C using my own drivers and prints the values over USB/UART.
A Python test system checks the data using pyserial and pytest.
I broke the setup on purpose to see if the test system could detect the faults.
I also used a logic analyzer to capture the bus traffic, including the faults and verify that the drivers behave as expected.

## Hardware
| Component | Notes |
|---|---|
| Raspberry Pi Pico 2 WH | RP2350 (Cortex-M33), 3.3 V logic |
| BME280 breakout | I2C, on-board 4.7 kΩ pull-ups |
| Logic analyzer | 8 ch, 24 MHz, PulseView/sigrok |

## Wiring
TODO: table Pico pin ↔ BME280 / analyzer channel

## Architecture
TODO: BME280 → I2C → Pico → UART/USB → Python test → report

## Build and Flash
| Tool | Version |
|---|---|
| Pico SDK | 2.3.1 |
| ARM GCC | 13.2.Rel1 |
| CMake | 4.3.4 |
| Ninja | 1.13.2 |

Target board: `pico2_w`

1. Open the `firmware/` folder in VS Code with the Raspberry Pi Pico extension installed. On first open the extension downloads the SDK and toolchain.
2. Click **Compile** in the status bar. The output ends up in `firmware/build/`, e.g. `blink.uf2`.
3. Hold BOOTSEL, plug in USB, and copy the `.uf2` to the `RP2350` drive. The board reboots and starts the program.

## Bus Analysis
### UART
`printf` output goes to the USB CDC and UART0 (TX = GPIO 0, 115200 baud 8N1).
The USB side shows up as a virtual COM port, so the UART signal on GPIO 0 can be captured with a logic analyzer.

**Setup**
| Analyzer | Pico |
|---|---|
| D0 | GPIO 0 (pin 1, UART TX) |
| GND | GND (pin 3) |

PulseView with the UART decoder (115200, 8N1), sample rate 2 MHz.

![UART capture](docs/captures/uart_blink_line.png)

One line of the blink output (`Blink count: 69`) decoded in PulseView.

**Observations**
- Each byte: start bit (low), 8 data bits LSB first, stop bit (high). Line idle: High.
- Bit order checked on the '9' (0x39 → `1001 1100` on the wire).
- The line ends with `0D 0A`, although the code only sends `\n`.
- Measured bit time: 8.50 µs (expected 1/115200 ≈ 8.68 µs).

Raw capture: [uart_blink.sr](docs/captures/uart_blink.sr) (open with PulseView)

### I2C
TODO

## Test Concept
TODO

## Fault Injection
| Fault | Expected | Result | Capture |
|---|---|---|---|
| TODO | | | |

## Lessons Learned
- Checking the bit order with the letter 'B' was pointless since 0x42 is symmetric in binary.
The digit '9' (0x39) shows it properly.
- printf only sends '\n' but the capture shows '0D 0A'. The SDK adds a CR before the LF.