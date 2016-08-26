#!/bin/sh

# Exit immediately on failure.
set -e

echo "Running 'autoreconf -vi'..."
echo ""
autoreconf -vi

echo ""
echo "Done.  To compile and install AlPACA, execute:"
echo "   cd build"
echo "   ../configure"
echo "   sudo make install"
echo ""
