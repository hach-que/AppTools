from setuptools import setup
from distutils.sysconfig import get_python_lib
import glob
import os
import sys

if os.path.exists("README.md"):
    print("""The setup.py script should be executed from the build directory.

Please see the file 'README.md' for further instructions.""")

setup(
    name = "apptools",
    package_dir = {'': 'appfs/libapp/python'},
    data_files = [(get_python_lib()],
    author = "James Rhodes",
    description = "AppTools infrastructure for package management.",
    license = "MIT",
    keywords = "apptools package management",
    url = "https://github.com/hach-que/AppTools",
    )
