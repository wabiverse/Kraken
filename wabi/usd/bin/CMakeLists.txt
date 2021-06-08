set(DIRS
    usdcat
    usdchecker
    usdcompress
    usddiff
    usddumpcrate
    usdedit
    usdresolve
    usdstitch
    usdstitchclips
    usdzip
    sdfdump
    sdffilter
    usdtree
)

foreach(d ${DIRS})
    add_subdirectory(${d})
endforeach()
