Import("env")

# please keep $SOURCE variable, it will be replaced with a path to firmware

# Generic
env.Replace(
    UPLOADER="avrdude",
    UPLOADCMD="$UPLOADER $UPLOADERFLAGS -Uflash:w:${SOURCE}"
)