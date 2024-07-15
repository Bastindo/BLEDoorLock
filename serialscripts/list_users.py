import serial
import time
from dotenv import load_dotenv
import os
from user import User
from send_command import send_command

# Load environment variables from .env file
load_dotenv()

# Configuration
LIST_USERS_CMD = "cat users.csv"
DEBUG = os.getenv('DEBUG', 'false').lower() == 'true'

def list_users():
    user_list = send_command(LIST_USERS_CMD, 5)
    users = []
    if user_list:
        user_list = "\n".join(line for line in user_list.split("\n") if not line.startswith(">") and LIST_USERS_CMD not in line)
        users = [User.from_csv_line(line) for line in user_list.split('\n') if line.strip()]
    return users

if __name__ == "__main__":
    users = list_users()
    print(f"Users: {[str(f'{user.username}:{user.role}') for user in users]}")
