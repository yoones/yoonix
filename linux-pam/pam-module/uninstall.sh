#!/bin/bash

set -e

make fclean -C ./module
rm -f /tmp/libdumb_test.so

sudo rm -f ./program/dumb_test
make fclean -C ./program

sudo rm /etc/pam.d/dumb_test
