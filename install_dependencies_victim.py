import subprocess
import sys
import os
import webbrowser
import requests
import zipfile
import shutil
import tempfile
from pathlib import Path
import platform
import ctypes

# Python packages required for the victim projects
PYTHON_PACKAGES = [
    "paho-mqtt",        # MQTT client library
    "flask",            # Web framework for dashboards
    "flask-socketio",   # WebSocket support for real-time updates
    "python-dotenv",    # Environment variable management
    "requests",         # HTTP library for API calls
]

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin() != 0
    except:
        return False

def run_as_admin():
    """Re-run the script with administrator privileges."""
    if not is_admin():
        print("Requesting administrator privileges...")
        # Re-run the program with admin rights
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, " ".join(sys.argv), None, 1)
        sys.exit()

def check_chocolatey():
    try:
        subprocess.run(['choco', '--version'], check=True, capture_output=True)
        return True
    except (subprocess.SubprocessError, FileNotFoundError):
        return False

def install_chocolatey():
    print("\nInstalling Chocolatey package manager...")
    try:
        # Create a temporary PowerShell script with the installation commands
        install_script = """
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
        """
        
        # Create a temporary PowerShell script
        with tempfile.NamedTemporaryFile(suffix='.ps1', delete=False) as temp:
            temp.write(install_script.encode())
            temp_path = temp.name

        # Run the PowerShell script with elevated privileges
        subprocess.run(['powershell', '-ExecutionPolicy', 'Bypass', '-File', temp_path], check=True)
        
        # Clean up the temporary script
        os.unlink(temp_path)
        
        # Verify installation
        if check_chocolatey():
            print("Chocolatey installed successfully!")
            return True
        else:
            print("Chocolatey installation completed but verification failed.")
            return False
    except Exception as e:
        print(f"Error installing Chocolatey: {e}")
        return False

def check_openssl():
    try:
        subprocess.run(['openssl', 'version'], check=True, capture_output=True)
        return True
    except (subprocess.SubprocessError, FileNotFoundError):
        return False

def install_openssl():
    print("\nInstalling OpenSSL using Chocolatey...")
    try:
        # Run choco install with elevated privileges
        subprocess.run(['choco', 'install', 'openssl', '-y', '--force'], check=True)
        
        # Refresh environment variables
        os.environ['PATH'] = subprocess.check_output(['powershell', '-Command', '[Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [Environment]::GetEnvironmentVariable("Path", "User")']).decode().strip()
        
        # Verify installation
        if check_openssl():
            print("OpenSSL installed successfully!")
            return True
        else:
            print("OpenSSL installation completed but verification failed.")
            return False
    except subprocess.SubprocessError as e:
        print(f"Error installing OpenSSL: {e}")
        return False

def check_mosquitto():
    try:
        subprocess.run(['mosquitto', '-v'], check=True, capture_output=True)
        return True
    except (subprocess.SubprocessError, FileNotFoundError):
        return False

def install_mosquitto():
    print("\nInstalling Mosquitto using Chocolatey...")
    try:
        # Run choco install with elevated privileges
        subprocess.run(['choco', 'install', 'mosquitto', '-y', '--force'], check=True)
        
        # Refresh environment variables
        os.environ['PATH'] = subprocess.check_output(['powershell', '-Command', '[Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [Environment]::GetEnvironmentVariable("Path", "User")']).decode().strip()
        
        # Verify installation
        if check_mosquitto():
            print("Mosquitto installed successfully!")
            return True
        else:
            print("Mosquitto installation completed but verification failed.")
            return False
    except subprocess.SubprocessError as e:
        print(f"Error installing Mosquitto: {e}")
        return False

def check_python_installed():
    """Check if Python is installed and accessible."""
    try:
        subprocess.run([sys.executable, "--version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return True
    except:
        return False

def install_python_package(package_name):
    """Installs a Python package using pip."""
    print(f"Installing Python package '{package_name}'...")
    try:
        subprocess.run([sys.executable, "-m", "pip", "install", "--user", package_name], check=True)
        print(f"'{package_name}' installed successfully.")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error installing '{package_name}'. Command failed with exit code {e.returncode}.")
        return False

def main():
    """Main function to check and install all required packages."""
    print("\n=== PFA Project: IoT Cybersecurity Tool (Victim) Dependencies Check for Windows ===")
    print("This script will ensure all necessary packages are installed for the victim projects.")
    print("IMPORTANT: This script requires administrator privileges.")
    print("If you see a UAC prompt, please click 'Yes' to allow the installation.")
    print("================================================================\n")

    # Request admin privileges if not already running as admin
    if not is_admin():
        print("This script needs administrator privileges to install packages.")
        print("Attempting to restart with elevated privileges...")
        run_as_admin()
        return

    # Check and install Chocolatey if needed
    if not check_chocolatey():
        print("Chocolatey not found. Installing...")
        if not install_chocolatey():
            print("\nFailed to install Chocolatey automatically.")
            print("Please follow these steps manually:")
            print("1. Open PowerShell as Administrator")
            print("2. Run this command:")
            print('   Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString("https://community.chocolatey.org/install.ps1"))')
            print("3. After installation, close and reopen PowerShell as Administrator")
            print("4. Run this script again")
            sys.exit(1)
    else:
        print("Chocolatey is already installed.")

    # Check and install OpenSSL
    if not check_openssl():
        if not install_openssl():
            print("\nFailed to install OpenSSL automatically.")
            print("Please follow these steps manually:")
            print("1. Open PowerShell as Administrator")
            print("2. Run: choco install openssl -y")
            print("3. Close and reopen PowerShell as Administrator")
            print("4. Run this script again")
            sys.exit(1)
    else:
        print("OpenSSL is already installed.")

    # Check and install Mosquitto
    if not check_mosquitto():
        if not install_mosquitto():
            print("\nFailed to install Mosquitto automatically.")
            print("Please follow these steps manually:")
            print("1. Open PowerShell as Administrator")
            print("2. Run: choco install mosquitto -y")
            print("3. Close and reopen PowerShell as Administrator")
            print("4. Run this script again")
            sys.exit(1)
    else:
        print("Mosquitto is already installed.")

    # Check Python installation
    if not check_python_installed():
        print("\nPython is not installed or not in PATH.")
        print("Please follow these steps:")
        print("1. Download Python from https://www.python.org/downloads/")
        print("2. During installation, make sure to check 'Add Python to PATH'")
        print("3. After installation, close and reopen PowerShell as Administrator")
        print("4. Run this script again")
        webbrowser.open("https://www.python.org/downloads/")
        sys.exit(1)

    # Install Python packages
    print("\nInstalling Python packages...")
    all_dependencies_met = True
    for package in PYTHON_PACKAGES:
        if not install_python_package(package):
            print(f"!!! Failed to install Python package '{package}'. Please address this issue manually. !!!")
            all_dependencies_met = False

    print("\n=================================================================")
    if all_dependencies_met:
        print("✅ All required dependencies are installed successfully!")
        print("You can now proceed to deploy the victim projects!")
        print("Example: cd VictimProjects && python deploy.py")
    else:
        print("❌ Some dependencies could not be installed.")
        print("Please review the output above and manually resolve any issues.")
        sys.exit(1)
    print("=================================================================\n")

if __name__ == "__main__":
    main() 