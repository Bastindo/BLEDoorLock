import os
from dotenv import load_dotenv
import serial
import time
import sys
from list_users import list_users
from user import User
from send_command import send_command

# Load environment variables from .env file
load_dotenv()

def user_exists(username):
    users = list_users()
    return any(user.username == username for user in users)

def remove_user(username):
    remove_user_cmd = f"removeuser {username}"
    if user_exists(username):
        send_command(remove_user_cmd)
        print(f"User '{username}' removed successfully.")
    else:
        print(f"User '{username}' does not exist in the database.")

def main(input_arg):
    if os.path.isfile(input_arg):
        with open(input_arg, 'r') as file:
            for username in file:
                username = username.strip()
                if username:
                    remove_user(username)
    else:
        remove_user(input_arg)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python remove_users.py <username_or_user_list_file>")
        sys.exit(1)

    input_arg = sys.argv[1]
    main(input_arg)
