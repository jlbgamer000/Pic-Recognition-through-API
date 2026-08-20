function hi() {
    echo "1. Stopping and masking arduino-router..."
    sudo systemctl stop arduino-router
    sudo systemctl mask arduino-router

    echo "2. Clearing port locks and zombie processes..."
    # The '|| true' ensures the script doesn't abort if the port is already clean
    sudo fuser -k /dev/ttyHS1 2>/dev/null || true
    sudo killall -9 gpioset python3 2>/dev/null || true

    echo "3. Syncing system clock for SSL verification..."
    # You must update this date string if the board is powered off for days
    sudo date -s "20 AUG 2026 08:43:00"

    echo "4. Reloading systemd and launching the bridge..."
    sudo systemctl daemon-reload && \
    sudo systemctl enable --now fridge-bridge.service && \
    sudo systemctl status fridge-bridge.service --no-pager && \
    echo "5. Bridge is live! Streaming logs..." && \
    journalctl -u fridge-bridge -f
}