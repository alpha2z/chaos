#!/bin/sh
find . -name "*.h" -o -name "*.cpp" -o -name "*.c" -o -name "*.cc" -o -name "*.hpp" > cscope.files 
cscope -bkq -i cscope.files 
ctags -R
