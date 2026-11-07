# Pico BME280 Test System
**Status:** UART output and capture working. Button interrupt with debouncing done and tested. BME280 driver and tests in progress.

I wrote this project to get practical experience with C on a microcontroller and with bus protocols
I had only seen in lectures. A Pico 2 WH reads a BME280 sensor over I2C using my own driver and prints the values over USB/UART.
A Python test system checks the data using pyserial and pytest.
I broke the setup on purpose to see if the test system could detect the faults.
I also used a logic analyzer to capture the bus traffic, including the faults and verify that the driver behaves as expected.

## Hardware
| Component | Notes |
|---|---|
| Raspberry Pi Pico 2 WH | RP2350 (Cortex-M33), 3.3 V logic |
| BME280 breakout | I2C, on-board 4.7 kΩ pull-ups |
| Logic analyzer | 8 ch, 24 MHz, PulseView/sigrok |
| Push button, LED, 330 Ω | user input and feedback |

## Wiring
| Pico | Connected to | Analyzer |
|---|---|---|
| GPIO 0 (pin 1) | UART TX | D0 |
| GPIO 14 (pin 19) | LED → 330 Ω → GND | – |
| GPIO 15 (pin 20) | push button → GND (internal pull-up) | D0 (during button tests) |
| GND (pin 3 / 18) | common ground | GND |
| TODO | BME280 SDA / SCL / 3V3 / GND | |

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
2. Click **Compile** in the status bar. The output ends up in `firmware/build/`, e.g. `c_pico.uf2`.
3. Hold BOOTSEL, plug in USB, and copy the `.uf2` to the `RP2350` drive. The board reboots and starts the program.

The current state is always in `firmware/c_pico.c`. Earlier working steps are kept in `firmware/experiments/` for reference. They are not part of the build.

## Bus Analysis
### UART
`printf` output goes to the USB CDC and UART0 (TX = GPIO 0, 115200 baud 8N1).
The USB side shows up as a virtual COM port.
The UART signal on GPIO 0 can be captured with a logic analyzer.

**Setup**
| Analyzer | Pico |
|---|---|
| D0 | GPIO 0 (pin 1, UART TX) |
| GND | GND (pin 3) |

PulseView with the UART decoder (115200, 8N1), sample rate 2 MHz.

![UART capture](docs/captures/1_uart_blink_line.png)

One line of the blink output (`Blink count: 69`) decoded in PulseView.

**Observations**
- Each byte: start bit (low), 8 data bits LSB first, stop bit (high). Line idle: High.
- Bit order checked on the '9' (0x39 → `1001 1100` on the wire).
- The line ends with `0D 0A`, although the code only sends `\n`.
- Measured bit time: 8.50 µs (expected 1/115200 ≈ 8.68 µs). At 2 MHz one sample is 0.5 µs, so the difference is within the measurement resolution.

Raw capture: [uart_blink.sr](docs/captures/1_uart_blink.sr) (open with PulseView)

### I2C
TODO

## Button Interrupt and Debouncing
A push button on GPIO 15 triggers a falling-edge interrupt. The ISR only sets a `volatile` flag, the main loop does the actual work (count, toggle LED, print).

**Why pull-up and button to GND:** My board has the RP2350 A2 stepping, which is affected by erratum RP2350-E9.
With a weak internal pull-down a pin can get stuck at around 2 V after being driven high. A pull-up with the button to GND avoids the problem.

### Measuring the bounce
Without debouncing, 20 presses gave 24 counts. At first I could not see any bouncing in PulseView, because I sampled at 20 kHz and the pulses are too short.
At 8 MHz the capture shows the problem: the button bounces when it is released, but not when it is pressed.

![Release bounce overview](docs/captures/2_button_raw_bounce_overview.png)
![Release bounce zoom](docs/captures/2_button_raw_bounce_zoom.png)

- Bounce duration on release: ~0.42 ms
- 3 short high pulses (~1 µs to ~22 µs), each one ends with a falling edge → 3 false interrupts
- PulseView counter decoder jumps from 10 to 13 on a single release

### Debouncing
A simple time lock after the first edge would not work here. If the button is held longer than the lock time, the release bounce is counted as a new press.
I implemented a simple debouncing method using a one-shot alarm. The ISR disables the pin interrupt and starts a timer. The callback reads the pin and decides if it was a real press or just a bounce. Then the interrupt is enabled again.

1. Falling edge → ISR disables the pin interrupt and starts a one-shot alarm (T = 10 ms).
2. Alarm callback reads the pin: LOW = real press, HIGH = release bounce → ignored.
3. The interrupt is enabled again.

T has to be longer than the bounce and shorter than the shortest real press:
0.42 ms < T < ~50 ms. I chose 10 ms, so there is a large margin to both sides.

If no alarm slot is free, the ISR enables the interrupt again right away. The button recovers on its own after a slot is free. This is a rare case. I stumbled upon it when I checked the possible return values of `add_alarm_in_ms()`.

### Results
| Test | Expected | Result |
|---|---|---|
| 20 presses, short and long mixed | press = 20 | ✓ 20 |
| Hold 5 s, then release | press +1 | ✓ +1 |
| Scratch test (artificial bounce) | analyzer edges > ISR calls > presses | ✓ 93 > 29 > 19 |

During the first two tests no release bounced, so the filter was not really tested. To trigger it on purpose, I scratched a GND wire across GPIO 15:

![Scratch test](docs/captures/2_button_scratch_test.png)

Logs: [docs/logs/](docs/logs/)

## Test Concept
TODO

## Fault Injection
| Fault | Expected | Result | Capture |
|---|---|---|---|
| Artificial bounce on button pin | bounce edges filtered, ISR calls < edges | 93 edges → 29 ISR → 19 presses | [scratch test](docs/captures/2_button_scratch_test.png) |


## Lessons Learned
- Checking the bit order with the letter 'B' was pointless since 0x42 is symmetric in binary.
The digit '9' (0x39) shows it properly.
- printf only sends '\n' but the capture shows '0D 0A'. The SDK adds a CR before the LF.
- At 20 kHz the button bounce was not visible. At 8 MHz it is clearly visible.
