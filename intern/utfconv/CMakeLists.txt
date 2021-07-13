set(INC
  .
)

set(INC_SYS

)

set(SRC
  utfconv.cpp

  utfconv.h
)

set(LIB
)

kraken_add_lib(kraken_intern_utfconv "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")