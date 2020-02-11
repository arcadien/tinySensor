Import("env")

# Specific setup for all AVR target
if(env["PIOPLATFORM"] == "atmelavr"): 
    env.Replace(
        UPLOADER="avrdude",
        UPLOADCMD="$UPLOADER $UPLOADERFLAGS -Uflash:w:${SOURCE}"
    )