#!/bin/sh
# Q36(b): Create a named pipe with the mkfifo command.
# mkfifo creates a named pipe directly, without needing a file-type argument.
# The pipe will remain in the directory after this script finishes.
set -eu
mkfifo -m 600 q36_command_mkfifo
# Check the file type and permissions of the newly created FIFO.
ls -l q36_command_mkfifo
# Cleanup: rm q36_command_mkfifo

