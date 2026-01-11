
git clone --recursive https://github.com/cimgui/cimgui.git
cd cimgui
git submodule update --init --recursive

mingw32-make all CXX=g++ OS=Windows_NT