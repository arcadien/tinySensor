# Step debugging (debugWIRE)

The old Atmel Studio project (`TinySensor/`) was removed. Debugging now goes through PlatformIO + AVaRICE + AVR Dragon, over debugWIRE.

> Status: **not validated on the bench**. Procedure to confirm on the first try.

## Toolchain

| Item | Role |
|---|---|
| AVR Dragon | Probe (USB) — debugWIRE, ISP, HVSP |
| [AVaRICE](https://avarice.sourceforge.io/) | GDB server for the Dragon (`--dragon --debugwire`) |
| PlatformIO | `debug_tool = custom`, connects to the GDB server |
| avrdude | Fuses (enable/disable DWEN) |

Bloom is not an option: it targets EDBG probes (Atmel-ICE, Snap, PICkit 4), not the Dragon. `avr-stub` (PlatformIO native debugger) only supports ATmega parts.

To check: the ATtiny84 is in AVaRICE's debugWIRE target list (`avarice --known-devices`, `+` marker).

## PlatformIO environment (proposal)

```ini
[env:debug]
extends = env:devmodule      ; base env to choose
build_type = debug
debug_tool = custom
debug_port = :4242
debug_server = avarice --dragon --debugwire --part attiny84 :4242
debug_init_cmds =
  target remote $DEBUG_PORT
  file "$PROG_PATH"
  load
```

Start AVaRICE by hand if needed: `avarice --dragon --debugwire --part attiny84 :4242`.

## DWEN fuse

### Enable debugWIRE
Program DWEN (= 0) in the hfuse over ISP (`-c dragon_isp`). Once active, **/RESET is no longer a reset pin and ISP no longer works**.

### Back to ISP (no high voltage)
1. `avrdude -c dragon_dw -P usb -p t84` — avrdude connects over debugWIRE and switches the target to ISP mode, **without a power cycle**.
2. `avrdude -c dragon_isp -P usb -p t84 -U hfuse:w:0xXX:m` — rewrite the hfuse with DWEN = 1 (start from the current value).

AVaRICE cannot disable DWEN itself. avrdude sequence to validate depending on the version.

### Fallback: high voltage (HVSP)
If debugWIRE no longer answers: HVSP at 12 V (tolerance ~11.5–12.5 V), supported by the Dragon, dedicated wiring.

## debugWIRE limits

- No hardware breakpoint: software breakpoints by rewriting flash pages (wear).
- Fuses and lock bits unreachable during the session.
- Chip erase (`-e`) impossible in debugWIRE mode.

## Hardware constraints — dedicated debug board

Production boards are not designed for debugWIRE. A dedicated debug board is needed with:

- **/RESET without a capacitor** (the line carries the debugWIRE signal).
- **/RESET pull-up ≥ 10 kΩ** (or no external resistor; do not use a lower value).
- Reachable 6-pin ISP connector (enable/disable DWEN).
- Ideally: access to the HVSP wiring for recovery.
