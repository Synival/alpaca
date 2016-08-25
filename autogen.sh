#!/bin/sh

# Exit immediately on failure.
set -e

echo "Running 'autoreconf -vi'..."
echo ""
autoreconf -vi

echo ""
echo "Done.  To compile and install AlPACA, execute:"
echo "   ./configure"
echo "   sudo make install"
