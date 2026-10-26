#!/bin/sh
# Q36(a): Create a named pipe with the mknod command.
# The letter p tells mknod to create a FIFO rather than a device file.
# Permission 600 lets only the owner read from and write to this pipe.
set -eu
mknod -m 600 q36_command_mknod p
# A p at the start of the listing confirms that this is a named pipe.
ls -l q36_command_mknod
# Cleanup: rm q36_command_mknod

