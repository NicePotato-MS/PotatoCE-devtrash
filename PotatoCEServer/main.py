import asyncio
import threading
import serial
import socket
import requests
import json

# Serial communication settings
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

# UDP server settings
UDP_HOST = '0.0.0.0'
UDP_PORT = 38600

poll_cache = []

# Mutex to ensure thread-safe serial communication
serial_lock = threading.Lock()

# Initialize the serial connection with '\0' as the break character
serial_port = None

# Function to initialize the serial connection asynchronously
async def init_serial():
    global serial_port
    print("Serial device is asleep.")
    while not serial_port:
        try:
            serial_port = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1, stopbits=serial.STOPBITS_TWO)
            print("Serial device connected.")
        except serial.SerialException:
            await asyncio.sleep(1)

# Function to read from the serial device and update poll_cache
async def serial_reader():
    global serial_port
    global poll_cache  # Declare poll_cache as a global variable
    while True:
        with serial_lock:
            if serial_port:
                try:
                    data = serial_port.read_until(b'\0').decode().strip()
                    if data == "MEACK":
                        # Update the poll_cache with the latest poll info
                        poll_cache.append("CLACK")
                        print("poll!")
                except serial.SerialException:
                    print("Serial communication error. Reconnecting...")
                    if serial_port:
                        serial_port.close()
                        serial_port = None
                    await init_serial()
            else:
                if serial_port:
                        serial_port.close()
                        serial_port = None
                await init_serial()
        await asyncio.sleep(0.05)
        
def serOut(str):
    global serial_port
    with serial_lock:
        if serial_port:
            try:
                out = str
                serial_port.write(out.encode())
            except serial.SerialException:
                print("Serial communication error while writing UDP data.")
                if serial_port:
                    serial_port.close()
                    serial_port = None
        
def ping_webhook(params, webhook_url="https://trigger.macrodroid.com/73e7f2dc-80be-4e1d-b47c-ce56267718b7/potatoce"):
    try:
        response = requests.get(webhook_url, params=params)

        # Check if the request was successful (status code 2xx)
        if 200 <= response.status_code < 300:
            print("Ping sent to webhook successfully.")
            return True
        else:
            print(f"Failed to send ping to webhook. Status code: {response.status_code}")
            return False
    except Exception as e:
        print(f"Error sending ping to webhook: {e}")
        return False

# Function to handle UDP requests
async def udp_server():
    loop = asyncio.get_event_loop()
    transport, protocol = await loop.create_datagram_endpoint(
        lambda: UdpProtocol(),
        local_addr=(UDP_HOST, UDP_PORT)
    )

    # Get the LAN and WAN addresses of the UDP server
    lan_address = socket.gethostbyname(socket.gethostname())
    wan_address = socket.gethostbyname(socket.getfqdn())
    print(f"UDP server started. LAN address: {lan_address}, WAN address: {wan_address}")

# UDP protocol class
class UdpProtocol:
    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data, addr):
        # Handle UDP data received
        udp_data = data.decode()
        print(f"Received UDP data: {udp_data}")

        # Write UDP data to the serial port (requires serial_lock)
        serOut(udp_data)

# Main function to start the event loop
async def main():
    # Start serial reader and UDP server concurrently
    serial_task = asyncio.create_task(serial_reader())
    udp_task = asyncio.create_task(udp_server())

    # Wait for both tasks to complete
    await asyncio.gather(serial_task, udp_task)

if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
