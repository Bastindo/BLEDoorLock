import os
import sys
import random
import string
import json
import qrcode
import tempfile
from dotenv import load_dotenv
from list_users import list_users
from send_command import send_command
from PIL import Image
import matplotlib.pyplot as plt

# Load environment variables from .env file
load_dotenv()

LOCK_ID = os.getenv('LOCK_ID', 'default_lock_id')  # Replace with your actual lock ID
DEBUG = os.getenv('DEBUG', 'false').lower() == 'true'

def generate_password(length=12):
    characters = string.ascii_letters + string.digits
    password = ''.join(random.choice(characters) for i in range(length))
    return password

def user_exists(username):
    users = list_users()
    return any(user.username == username for user in users)

def add_user(username):
    password = generate_password()
    add_user_cmd = f"adduser {username} {password}"
    
    if user_exists(username):
        print(f"User '{username}' already exists in the database.")
    else:
        send_command(add_user_cmd)
        print(f"User '{username}' added successfully.")
        
        json_data = {
            "lockId": LOCK_ID,
            "password": password,
            "userName": username,
            "lockName": "NimBLE Servo Lock 2",
            "isAdmin": False,
            "color": None
        }
        
        json_string = json.dumps(json_data)
        if DEBUG:
            print(f"JSON Data: {json_string}")
        
        # Create a temporary file for the QR code
        with tempfile.NamedTemporaryFile(delete=False, suffix=".png") as temp_qr_file:
            qr_code = qrcode.make(json_string)
            qr_code.save(temp_qr_file.name)
            qr_code_file = temp_qr_file.name
        
        print("QR Code generated.")
        
        try:
            # Display the QR code image using Pillow and matplotlib
            img = Image.open(qr_code_file)
            plt.imshow(img)
            plt.axis('off')
            plt.show()
            
            # Wait for the user to close the image viewer
            
            #input("Press Enter after scanning the QR code to delete the file...")
        except Exception as e:
            print(f"Error displaying QR code: {e}")
        finally:
            # Delete the QR code file
            os.remove(qr_code_file)
            print("QR Code file deleted.")

def main(username):
    add_user(username)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python add_user.py <username>")
        sys.exit(1)

    username = sys.argv[1]
    main(username)
