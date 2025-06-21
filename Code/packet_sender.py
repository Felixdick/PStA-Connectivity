import socket
import subprocess
import platform

# --- Configuration ---
DEVICE_IP = "192.168.1.100"  
UDP_PORT = 60000             

# --- ADAPTER CONFIGURATION ---
# Set this to the IP of the network adapter on your PC you want to use.
# To automatically use the best adapter, leave this as "0.0.0.0".
SOURCE_IP = "192.168.1.1"       

# --- Build the CAN message payload ---
# Version: 1, Channel: 1
payload = bytearray([0x01, 0x01])

# CAN ID: 0x7DF (as a 32-bit little-endian integer)
can_id = 0x7DF
payload.extend(can_id.to_bytes(4, 'little'))

# ID Type: 0 (Standard), Frame Type: 0 (Data), DLC: 8
payload.extend([0x00, 0x00, 0x08])

# Data: [0x02, 0x10, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00]
can_data = bytearray([0x55, 0x44, 0x50, 0x54, 0x4F, 0x43, 0x41, 0x4E])
payload.extend(can_data)

# --- Send the UDP packet ---
print(f"Sending {len(payload)} bytes to {DEVICE_IP}:{UDP_PORT}")
print(f"Payload (hex): {payload.hex().upper()}")

try:
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # If a specific source IP is set, bind the socket to it
    if SOURCE_IP != "0.0.0.0":
        print(f"Using network adapter with IP: {SOURCE_IP}")
        # Binding to port 0 tells the OS to pick any available ephemeral port
        sock.bind((SOURCE_IP, 0))
    else:
        print("Using default network adapter chosen by OS.")

    # Send the data
    sock.sendto(payload, (DEVICE_IP, UDP_PORT))
    print("Packet sent successfully!")
except Exception as e:
    print(f"Error sending packet: {e}")
finally:
    sock.close()