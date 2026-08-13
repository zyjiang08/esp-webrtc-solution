#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT="${1:-8010}"

cd "$SCRIPT_DIR"

if ! command -v python3 >/dev/null 2>&1; then
  echo "python3 not found"
  exit 1
fi

if [ ! -f "apprtc_test_server.py" ] || [ ! -f "minimal_test.html" ]; then
  echo "required files not found in $SCRIPT_DIR"
  exit 1
fi

echo "Minimal WebRTC test page"
echo ""
echo "Open in browser:"
echo "  http://127.0.0.1:${PORT}/minimal_test.html"
echo ""
echo "Suggested test flow:"
echo "  1. On the board console run: start room3322"
echo "  2. Open the page above"
echo "  3. Keep room id as room3322 and click Join Room"
echo "  4. If needed click Force Audio Play"
echo "  5. Check Log / Stats / Remote Audio panels"
echo ""
echo "Press Ctrl+C to stop the local server"
echo ""

exec python3 apprtc_test_server.py --port "$PORT"
