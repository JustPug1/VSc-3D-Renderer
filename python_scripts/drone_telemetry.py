import serial
import struct

# --- CONFIGURATION ---
SERIAL_PORT = 'COM5'
BAUD_RATE = 115200
MSP_ATTITUDE = 108

# Global serial connection
_ser = None

def init_serial():
    """Initializes the serial port once."""
    global _ser
    try:
        if _ser is None:
            _ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
            _ser.read_all() # Clear buffer
            print(f"[Python] Connected to {SERIAL_PORT}")
    except Exception as e:
        print(f"[Python] Serial Error: {e}")

def get_drone_attitude():
    """Requests data from FC and returns a dictionary for C++."""
    if _ser is None or not _ser.is_open:
        init_serial()
    
    # 1. Send MSP Request
    # $M< [size:0] [cmd:108] [checksum:108]
    _ser.write(b'$M<\x00l\x6c') 
    
    # 2. Read Response Header ($M>)
    header = _ser.read(3)
    if header == b'$M>':
        size_cmd = _ser.read(2)
        size, cmd = struct.unpack('<BB', size_cmd)
        data = _ser.read(size)
        _ser.read(1) # Read Checksum (skipping verification for brevity)

        if cmd == MSP_ATTITUDE and len(data) == 6:
            roll, pitch, yaw = struct.unpack('<hhh', data)
            
            # Return as a dictionary
            return {
                "roll": roll / 10.0,
                "pitch": pitch / 10.0,
                "yaw": float(yaw)
            }
            
    return {"roll": 0.0, "pitch": 0.0, "yaw": 0.0}