set(INC
  .
  ../editors/include
  ../wm
  ../io
  ../krakernel
  ../universe
)

set(SRC
  intern/SERVER_run.cpp

  SERVER_api.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${ZMQ_INCLUDE_DIR}
  ${PYTHON_INCLUDE_DIR}
)

set(LIB
  maelstrom
  kraken_wm
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${ZMQ_LIBRARIES}
)

kraken_add_lib(kraken_server "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")