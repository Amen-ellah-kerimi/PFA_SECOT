import subprocess
import sys
import os
import time

def run_command(command):
    """Run a shell command and print its output"""
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    for line in process.stdout:
        print(line, end='')
    process.wait()
    return process.returncode

def deploy_project(project, security_level):
    """Deploy a project with a specific security level"""
    print(f"\n{'='*80}")
    print(f"Deploying {project} with {security_level} security...")
    print(f"{'='*80}\n")
    
    # Change to the project directory
    os.chdir(f'VictimProjects/{project}')
    
    # Run the deployment script
    result = run_command(f'python3 ../deploy.py {security_level}')
    
    # Change back to the root directory
    os.chdir('../..')
    
    if result == 0:
        print(f"\n✓ Successfully deployed {project} with {security_level} security")
    else:
        print(f"\n✗ Failed to deploy {project} with {security_level} security")
    
    return result

def main():
    """Main function to deploy all projects with all security levels"""
    projects = ['WeatherStation', 'SmartLight']
    security_levels = ['no-security', 'basic-security', 'secure']
    
    print("\n=== PFA Project: IoT Cybersecurity Tool - Deployment ===")
    print("This script will deploy all projects with all security levels.")
    print("Each deployment will take a few minutes to complete.")
    print("You can access the web dashboards at:")
    print("- WeatherStation: http://localhost:8080")
    print("- SmartLight: http://localhost:8081")
    print("\nPress Ctrl+C at any time to stop the deployment process.")
    print("="*80)
    
    try:
        for project in projects:
            for security_level in security_levels:
                if deploy_project(project, security_level) != 0:
                    print(f"\nFailed to deploy {project} with {security_level} security.")
                    print("Please check the error messages above and try again.")
                    sys.exit(1)
                
                # Wait a bit between deployments to ensure clean state
                time.sleep(5)
        
        print("\n=== Deployment Complete ===")
        print("All projects have been deployed successfully!")
        print("\nYou can now:")
        print("1. Access the web dashboards at:")
        print("   - WeatherStation: http://localhost:8080")
        print("   - SmartLight: http://localhost:8081")
        print("2. Use MQTT clients to connect to the brokers:")
        print("   - WeatherStation: localhost:1883 (MQTT), localhost:9001 (WebSocket)")
        print("   - SmartLight: localhost:1884 (MQTT), localhost:9002 (WebSocket)")
        print("3. For secure connections:")
        print("   - WeatherStation: localhost:8883 (MQTT), localhost:9443 (WebSocket)")
        print("   - SmartLight: localhost:8884 (MQTT), localhost:9444 (WebSocket)")
        print("\nPress Ctrl+C to stop all services when you're done.")
        
    except KeyboardInterrupt:
        print("\n\nDeployment interrupted by user.")
        print("Stopping all services...")
        for project in projects:
            os.chdir(f'VictimProjects/{project}')
            run_command('docker-compose down')
            os.chdir('../..')
        print("All services stopped.")
        sys.exit(0)

if __name__ == "__main__":
    main() 