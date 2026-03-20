import serial
import struct

# --- CONFIGURATION ---
SERIAL_PORT = 'COM5'      # The active COM port for your Flight Controller
BAUD_RATE = 115200        # Standard baud rate for Betaflight/iNav USB connections
MSP_ATTITUDE = 108        # MSP command ID requesting roll, pitch, and yaw

# Global serial connection object. Keeping it global prevents the system 
# from constantly opening and closing the COM port every frame.
_ser = None

def init_serial():
    """
    Initializes the serial port connection. 
    Called automatically if the connection is missing or dropped.
    """
    global _ser
    try:
        if _ser is None:
            # timeout=0.1 ensures the program doesn't hang forever if the drone is disconnected
            _ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
            _ser.read_all()  # Flush any stale data sitting in the buffer
            print(f"[Python] Connected to {SERIAL_PORT}")
    except Exception as e:
        print(f"[Python] Serial Error: {e}")

def get_drone_attitude():
    """
    Constructs an MSP packet, sends it to the flight controller, 
    reads the response, and formats it into a dictionary for C++.
    """
    # Auto-reconnect logic
    if _ser is None or not _ser.is_open:
        init_serial()
    
    # --- 1. SEND MSP REQUEST ---
    # Packet Structure: Header ($M<) + Payload Size (0) + Command ID (108) + Checksum (108)
    # \x00 = Size 0. l = char for 108. \x6c = hex for 108 checksum.
    _ser.write(b'$M<\x00l\x6c') 
    
    # --- 2. READ MSP RESPONSE ---
    # Look for the response header ($M>) indicating incoming flight controller data
    header = _ser.read(3)
    if header == b'$M>':
        # Read the next 2 bytes: Payload Size and Command ID
        size_cmd = _ser.read(2)
        size, cmd = struct.unpack('<BB', size_cmd) # <BB = 2 unsigned bytes, Little Endian
        
        # Read the actual payload based on the size we just parsed
        data = _ser.read(size)
        _ser.read(1) # Read the Checksum byte (Skipping validation for performance)

        # Ensure we are looking at the correct response and the payload is exactly 6 bytes (3 x 16-bit ints)
        if cmd == MSP_ATTITUDE and len(data) == 6:
            # Unpack 6 bytes into three 16-bit signed integers (<hhh)
            roll, pitch, yaw = struct.unpack('<hhh', data)
            
            # MSP Attitude data for Roll/Pitch is scaled by 10 (e.g., 900 = 90.0 degrees)
            # Yaw is standard (-180 to 180)
            return {
                "roll": roll / 10.0,
                "pitch": pitch / 10.0,
                "yaw": float(yaw)
            }
            
    # If the packet was invalid, incomplete, or the port disconnected, return safe zero values
    return {"roll": 0.0, "pitch": 0.0, "yaw": 0.0}