#!/usr/bin/awk

# FS = :, some log file has the time stamp, that use ":"
BEGIN{CRASH_LINE = 0; FS = ":"}
$NF~/63430300/ {CRASH_LINE = 1}

{
    if (CRASH_LINE == 1) {
        if($0 ~ /End of crash dump/) {
            CRASH_LINE = 0;
            exit
        } else {
            print $0
        }
    }
}

