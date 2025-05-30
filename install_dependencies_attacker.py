import subprocess
import sys
import os

# List of Kali Linux packages for the attacker side
REQUIRED_PACKAGES = [
    "aircrack-ng",      # Airodump-ng, Aireplay-ng for Wi-Fi attacks
    "wireshark",        # Packet analysis
    "hydra",            # Password cracking
    "john",             # John the Ripper for password cracking
    "ettercap-graphical", # Ettercap for MITM
    "mdk4",             # For beacon flooding
    "wifiphisher",      # For advanced phishing attacks
    "mosquitto-clients",# For interacting with MQTT brokers
    "mosquitto",        # MQTT broker for testing
    "rust",             # For compiling the CLI tool
    "build-essential",  # Required for Rust compilation
    "pkg-config",       # Required for some Rust dependencies
    "libssl-dev",       # Required for SSL/TLS support
]

def check_package_installed(package_name):
    """Checks if a given Debian package is installed using dpkg-query."""
    try:
        result = subprocess.run(
            ["dpkg-query", "-W", "-f='${Status}'", package_name],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        return "install ok installed" in result.stdout
    except subprocess.CalledProcessError:
        return False
    except FileNotFoundError:
        print(f"Error: dpkg-query not found. Is this a Debian-based system (like Kali)?")
        sys.exit(1)

def install_package(package_name):
    """Attempts to install a given Debian package using apt-get."""
    print(f"Attempting to install '{package_name}'...")
    try:
        print("Updating package lists...")
        subprocess.run(["sudo", "apt-get", "update", "-y"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        subprocess.run(["sudo", "apt-get", "install", "-y", package_name], check=True)
        print(f"'{package_name}' installed successfully.")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error installing '{package_name}'. Command failed with exit code {e.returncode}.")
        print(f"Stderr: {e.stderr.decode(errors='ignore')}")
        print(f"Please ensure you have an active internet connection and that '{package_name}' is available in your Kali repositories.")
        return False
    except FileNotFoundError:
        print(f"Error: apt-get not found. Is this a Debian-based system (like Kali)?")
        sys.exit(1)

def main():
    """Main function to check and install all required packages."""
    if sys.platform != "linux":
        print("This script is designed for Linux systems (like Kali Linux) that use apt for package management.")
        sys.exit(1)

    if os.geteuid() != 0:
        print("This script requires root privileges to install packages.")
        print("Please run: sudo python3 install_dependencies_attacker.py")
        sys.exit(1)

    print("\n--- PFA Project: IoT Cybersecurity Tool (Attacker) Dependencies Check ---")
    print("This script will ensure all necessary Kali Linux tools are installed.")
    print("-----------------------------------------------------------\n")

    all_dependencies_met = True
    for package in REQUIRED_PACKAGES:
        if not check_package_installed(package):
            print(f"Missing dependency: '{package}'.")
            if not install_package(package):
                print(f"!!! Failed to install '{package}'. Please address this issue manually. !!!")
                all_dependencies_met = False
        else:
            print(f"Dependency '{package}' is already installed.")

    print("\n-----------------------------------------------------------")
    if all_dependencies_met:
        print("All required dependencies are installed. You can now proceed to compile and run your Rust CLI tool!")
        print("Example: cd cli_tool && cargo run -- [your commands]")
    else:
        print("Some dependencies could not be installed. Please review the output above and manually resolve any issues.")
        print("You may need to run 'sudo apt-get update' or check your internet connection.")
        sys.exit(1)
    print("-----------------------------------------------------------\n")

if __name__ == "__main__":
    main() 