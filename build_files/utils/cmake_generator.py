#!/usr/bin/python

import os
import sys


source_files   = []
source_headers = []
cmake_file = None
will_write = False

def FileWriter(write_file):
  f = open(write_file, "a")

  #LOCAL INCLUDE
  f.write(f"set(INC\n")
  f.write(f"  ..\n")
  f.write(f")\n")
  f.write(f"\n")

  #SOURCE CODE
  f.write(f"set(SRC\n")
  for file in source_files:
    f.write(f"  {file}\n")
  f.write(f")\n")

  f.write(f"\n")

  #SOURCE HEADERS
  f.write(f"set(SRC_HEADERS\n")
  for file in source_headers:
    f.write(f"  {file}\n")
  f.write(f")\n")

  f.write(f"\n")

  #LINKING LIBS
  f.write(f"set(LIB\n")
  f.write(f"  \n")
  f.write(f")\n")

  f.write(f"\n")

  #LOCAL INCLUDES
  f.write(f"include_directories(${{INC}})\n")
  #SYSTEM INCLUDES
  f.write(f"include_directories(SYSTEM ${{INC_SYS}})\n")

  f.write(f"\n")

  #ADD LIBRARY
  directory_name = os.path.dirname(write_file)
  f.write(f"wabi_add_library({os.path.basename(directory_name)} \"${{LIB}}\" ${{SRC}} ${{SRC_HEADERS}})\n")

  f.close()

  # Done.



def main(argv):
  rootdir = argv[0]

  for subdir, dirs, files in os.walk(rootdir):

    for file in files:

      if file.endswith(".h"):
        source_headers.append(file)

      if file.endswith(".cpp"):
        source_files.append(file)

      if file == "CMakeLists.txt":
        cmake_file = os.path.join(subdir, file)

      print(file)

  if cmake_file:
    print(f"Writing to {cmake_file}")
    FileWriter(cmake_file)



if __name__ == "__main__":
  main(sys.argv[1:])