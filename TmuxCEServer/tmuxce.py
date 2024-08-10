import subprocess
import os
import time
import asyncio
import aioconsole
import serial
import threading
import re

ROWS = 53
COLS = 24

def strip_ansi_codes(text):
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    return ansi_escape.sub('', text)

def tmux(command,text=True,check=True):
    return subprocess.run("tmux "+command, shell=True, text=text, check=check, capture_output=True)

def shell(command):
    return subprocess.run(command, shell=True,capture_output=True)

async def init_tmux():
    global dbg
    global buffer
    
    buffer_dir = "/tmp/tmuxce_buffer"
    if os.path.exists(buffer_dir):
        os.remove(buffer_dir)
        
    if os.path.exists("./debug.txt"):
        os.remove("./debug.txt")
    dbg = open("./debug.txt","a+")
    
    if tmux("has-session -t tmuxce",check=False).returncode == 0:
        tmux("kill-session -t tmuxce", check=False)
    tmux(f"new-session -d -c $HOME -x {ROWS} -y {COLS} -s tmuxce")
    tmux("new-window -d -c $HOME -n window1")

    shell("rm -rf /tmp/tmuxce_buffer")
    tmux(f"pipe-pane -o -t tmuxce:window1.0 'cat > {buffer_dir}'")
    buffer = open(buffer_dir, "a+")
    
    tmux("send-keys -t tmuxce:window1.0 'neofetch --off' C-m")
    #tmux("send-keys -t tmuxce:window1.0 'echo Hello, world!' C-m")
    #await asyncio.sleep(0.5)
    #result = tmux("capture-pane -p -t tmuxce:window1.0 -eCJ",text=True)
    #print(result.stdout)
    
    while True:
        command = await aioconsole.ainput("$>")
        tmux(f"send-keys -t tmuxce:window1.0 '{command}' C-m")


# Serial communication settings
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

# UDP server settings
UDP_HOST = '0.0.0.0'
UDP_PORT = 38600

poll_cache = []

# Mutex to ensure thread-safe serial communication
serial_lock = threading.Lock()
serial_port = None

async def init_serial():
    global serial_port
    print("Serial device is asleep.")
    while not serial_port:
        try:
            serial_port = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1, stopbits=serial.STOPBITS_TWO)
            print("Serial device connected.")
        except serial.SerialException:
            await asyncio.sleep(1)
        except KeyboardInterrupt:
            exit(1)

# Function to read from the serial device and update poll_cache
async def serial_reader():
    global serial_port
    global poll_cache
    #debugbuffer = open("./out.txt","r")
    while True:
        with serial_lock:
            if serial_port:
                try:
                    serial_port.write(bytes(buffer.read(),"utf8"))
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


async def main():
    # Start serial reader and UDP server concurrently
    tmux_task = asyncio.create_task(init_tmux())
    serial_task = asyncio.create_task(serial_reader())

    # Wait for both tasks to complete
    await asyncio.gather(tmux_task, serial_task)

if __name__ == "__main__":
    try:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(main())
    except KeyboardInterrupt:
        if serial_port:
            with serial_lock:
                serial_port.close()