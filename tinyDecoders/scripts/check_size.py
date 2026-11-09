# Post-build check of the attiny84a env (CONF-DECODE-001): PlatformIO only
# warns past board_upload.maximum_ram_size, so fail when .data + .bss exceed it.
import os
import subprocess

Import("env")

MAX_STATIC_RAM = int(env.GetProjectOption("board_upload.maximum_ram_size"))


def check_static_ram(source, target, env):
    elf = str(target[0])
    size = env.subst("$SIZETOOL")
    lines = subprocess.check_output([size, "-A", elf], text=True)
    sections = dict(
        (fields[0], int(fields[1]))
        for fields in (line.split() for line in lines.splitlines())
        if len(fields) >= 2 and fields[1].isdigit()
    )
    used = sections.get(".data", 0) + sections.get(".bss", 0)
    print("Static RAM (.data + .bss): %d / %d B" % (used, MAX_STATIC_RAM))
    if used > MAX_STATIC_RAM:
        print("Error: static RAM exceeds %d B" % MAX_STATIC_RAM)
        os.remove(elf)  # so the next build relinks and checks again
        env.Exit(1)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", check_static_ram)
