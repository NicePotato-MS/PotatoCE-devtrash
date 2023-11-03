import pty
import sys
import termios
import tty
import os

# Define the column and row sizes.
COLS = 40
ROWS = 5

def setup_terminal():
  """Sets up the terminal in raw mode so that we can read and write characters without echoing."""
  tty.setraw(sys.stdin.fileno())

def cleanup_terminal():
  """Restores the terminal to its original settings."""
  termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_tty)

def read_input():
  """Reads a line of input from the user."""
  return sys.stdin.readline().rstrip("\n")

def write_output(data):
  """Writes data to the terminal."""
  sys.stdout.write(data)

def main():
  """The main function that opens the bash terminal and handles input and output."""
  global old_tty

  setup_terminal()

  # Create a pseudo-terminal pair.
  master_fd, slave_fd = pty.openpty()

  # Spawn a bash shell in the pseudo-terminal.
  bash = pty.spawn(["/bin/bash"], master_fd)

  # Read and write input and output from the bash shell.
  while True:
    command = read_input()
    if not command:
      break

    write_output(command)
    write_output("\n")

    data = os.read(master_fd, 1024)
    write_output(data)

  # Clean up the terminal.
  cleanup_terminal()

  # Close the pseudo-terminal.
  os.close(master_fd)
  os.close(slave_fd)

if __name__ == "__main__":
  main()
