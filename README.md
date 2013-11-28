![Build Status](https://api.travis-ci.org/mmcilroy/hulk_core.png?branch=master)
  
https://travis-ci.org/mmcilroy/hulk_core

cppcheck --enable=warning --enable=performance --enable=information hulk_core

valgrind --leak-check=full --show-reachable=yes --error-exitcode=1 program
