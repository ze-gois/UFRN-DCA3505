# analysis module for fork_pid.c log files
import sys
import os

# Add the parent directory to the sys.path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Print module loading message
if __name__ == '__main__':
    print("analysis module loaded")