import sys
import os

build_dir = os.path.abspath("./build")  # Get absolute path
sys.path.append(build_dir)

import pybind

pybind.alloc()
pybind.erase()
