#!/bin/bash

set -e

make re -C ./module
cp ./module/libdumb_test.so /tmp/

make re -C ./program
sudo chown root ./program/dumb_test
sudo chmod +s ./program/dumb_test

sudo cp ./service/dumb_test /etc/pam.d/
