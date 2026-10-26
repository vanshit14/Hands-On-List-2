#!/bin/sh
# Q36(c): Compare actual syscall counts and timings with strace.
# strace must be installed before running this comparison.
set -eu
# Give each command its own new path so an existing FIFO cannot affect the test.
work=$(mktemp -d)
# Remove the two test FIFOs and their temporary directory when the script exits.
trap 'rm -f "$work/a" "$work/b"; rmdir "$work"' EXIT
# The -c option shows syscall counts, errors, and time spent in system calls.
echo "mknod syscall summary:"
strace -c mknod "$work/a" p
echo "mkfifo syscall summary:"
strace -c mkfifo "$work/b"
# On modern Linux both typically reach mknodat(2). Compare the summaries,
# repeating runs before drawing conclusions: startup and tracing add noise.
# There is no universal winner independent of implementation and environment.

