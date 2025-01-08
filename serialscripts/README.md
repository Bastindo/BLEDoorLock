# Serial Command Scripts for User Management

This repository contains scripts that can manage users for your application using serial connectivity, without a need for advanced Python knowledge.

## Before You Start 

Ensure that you have both Python and pip (a tool for installing Python packages) installed on your system. If not, you can download and install them from [Python's official site](https://www.python.org/downloads/).

## Setting Up The Project

1. **Download the Project Files**: You can download the project files from this page. Click on 'Code' and select 'Download ZIP'. Then, extract the ZIP file.

![Download ZIP](https://docs.github.com/assets/images/help/repository/code-button.png)

2. **Open the Terminal in the Project Folder**: This might be straightforward for Mac and Linux users - just open a terminal and navigate to the project folder. If you're on Windows, you can navigate to the project folder, click on the address bar and type `cmd`, and then press enter.

![Open terminal Windows](https://www.ionos.com/digitalguide/fileadmin/DigitalGuide/Screenshots_2018/Windows-Command-Prompt-run-cmd-in-folder.jpg)

3. **Install the Required Packages**: In the terminal, type `pip install -r requirements.txt` and press enter. This should install all the necessary packages.

## How to Use the Scripts

- **Adding a new user**: In the terminal, type `python add_user.py yourusername` and press enter. Replace `yourusername` with the username you want to add.

- **Removing a user**: To remove a user, type `python remove_user.py yourusername` and press enter. Replace `yourusername` with the username of the user you want to remove.

- **Listing all users**: To see all users, type `python list_users.py` and press enter. This will show you a list of all the users.

Remember, you need to be in the project directory in your terminal to run these commands.

## Need Help?

If you run into any problems or need more help, please feel free to create a new issue in this repository.

## License

This project is licensed under the MIT License.
