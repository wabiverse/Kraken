set(INC
  .
  ../editors/include
  ../wm
  ../io
  ../covakernel
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
  covahverse
  covah_wm
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${ZMQ_LIBRARIES}
)

covah_add_lib(covah_server "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")