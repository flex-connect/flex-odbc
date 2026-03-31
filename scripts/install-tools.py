import os
import secrets
import shutil
import string
import subprocess
import sys
import time
import zipfile

# --- Configuration ---
VS_CONFIG_PATH = os.path.join(os.getcwd(), ".vsconfig") 
VCPKG_ROOT = r"C:\vcpkg"
MAVEN_INSTALL_DIR = r"C:\Program Files\Maven"
MAVEN_VERSION = "3.9.14" # Updated to a stable version
MAVEN_URL = f"https://dlcdn.apache.org/maven/maven-3/{MAVEN_VERSION}/binaries/apache-maven-{MAVEN_VERSION}-bin.zip"

def run_command(cmd, info_msg=None):
    if info_msg:
        print(f"🚀 {info_msg}...")
    try:
        # shell=True is required for winget execution and path expansion
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        return result
    except Exception as e:
        print(f"❌ Execution Error: {e}")
        return None

def is_winget_success(result):
    """
    Returns True if winget succeeded OR if it returned 'No upgrade found'.
    Winget return code 0x8a15003f (-1978335169) means already up to date.
    """
    if not result:
        return False
    
    NO_UPGRADE_CODE = -1978335169 
    if result.returncode == 0 or result.returncode == NO_UPGRADE_CODE:
        return True
        
    output = (result.stdout + result.stderr).lower()
    success_keywords = [
        "no available upgrade found",
        "no newer package versions",
        "already installed"
    ]
    return any(key in output for key in success_keywords)

def install_or_update_app(name, tool_id):
    # 1. Check if tool is installed
    check_cmd = run_command(f"winget list --id {tool_id} --exact", f"Checking {name}")
    
    if check_cmd.returncode != 0:
        # 2. Not found -> Install
        install_res = run_command(
            f"winget install --id {tool_id} --source winget --exact --silent --accept-package-agreements --accept-source-agreements", 
            f"Installing {name}"
        )
        if install_res.returncode == 0:
            print(f"✅ {name} installed successfully.")
        else:
            print(f"❌ Failed to install {name}. Output: {install_res.stdout.strip()}")
    else:
        # 3. Found -> Try Upgrade
        upgrade_res = run_command(
            f"winget upgrade --id {tool_id} --source winget --exact --silent --accept-package-agreements", 
            f"Checking for updates to {name}"
        )
        
        if is_winget_success(upgrade_res):
            print(f"✅ {name} is up to date.")
        else:
            print(f"⚠️  {name} upgrade check encountered a notice.")

def install_maven_manual():
    """Downloads Maven via curl, extracts, and sets PATH."""
    if os.path.exists(os.path.join(MAVEN_INSTALL_DIR, "bin", "mvn.cmd")):
        print("✅ Maven is already installed manually.")
        return

    print(f"📦 Downloading Maven {MAVEN_VERSION}...")
    zip_path = os.path.join(os.environ["TEMP"], "maven.zip")
    
    # 1. Use curl to download
    run_command(f'curl -L "{MAVEN_URL}" -o "{zip_path}"', "Downloading Maven binary")
    
    # 2. Extract
    if not os.path.exists(MAVEN_INSTALL_DIR):
        os.makedirs(MAVEN_INSTALL_DIR)
    
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        # Extract to temp first to handle the internal folder name
        temp_extract = os.path.join(os.environ["TEMP"], "maven_tmp")
        zip_ref.extractall(temp_extract)
        inner_folder = os.path.join(temp_extract, f"apache-maven-{MAVEN_VERSION}")
        
        # Move contents to C:\Program Files\Maven
        for item in os.listdir(inner_folder):
            shutil.move(os.path.join(inner_folder, item), MAVEN_INSTALL_DIR)
            
    # 3. Add to System PATH (Permanently)
    maven_bin = os.path.join(MAVEN_INSTALL_DIR, "bin")
    run_command(f'setx /M PATH "%PATH%;{maven_bin}"', "Adding Maven to System PATH")
    print("✅ Maven manual installation complete.")

def setup_vcpkg():
    """Clones vcpkg to C:\vcpkg and runs bootstrap."""
    if os.path.exists(os.path.join(VCPKG_ROOT, "vcpkg.exe")):
        print("✅ vcpkg is already bootstrapped.")
        return

    if not os.path.exists(VCPKG_ROOT):
        run_command(f"git clone https://github.com/microsoft/vcpkg.git {VCPKG_ROOT}", "Cloning vcpkg")
    
    # Bootstrap
    os.chdir(VCPKG_ROOT)
    run_command(f".\\bootstrap-vcpkg.bat", "Bootstrapping vcpkg")
    
    # Optional: Integrate with Visual Studio
    run_command(f".\\vcpkg.exe integrate install", "Integrating vcpkg with Visual Studio")
    print("✅ vcpkg setup complete.")

    run_command(f'setx VCPKG_ROOT "{vcpkg_path}"', "Setting VCPKG_ROOT (User)")

def generate_strong_password(length=16):
    """Generates a password that satisfies SQL Server's complexity requirements."""
    alphabet = string.ascii_letters + string.digits + "!#%^*()-_=+"
    while True:
        password = ''.join(secrets.choice(alphabet) for i in range(length))
        # Ensure it has at least one of each required type
        if (any(c.islower() for c in password)
                and any(c.isupper() for c in password)
                and any(c.isdigit() for c in password)
                and any(c in "!#%^*()-_=+" for c in password)):
            return password

def deploy_sql_server(container_name="sql_server_dev"):
    # 1. Generate the password
    sql_password = generate_strong_password()
    print(f"🔐 Generated Strong Password: {sql_password}")

    # 2. Set to User Environment Variable (Permanent)
    # Using 'setx' without /M sets it for the User
    os.system(f'setx MSSQL_SA_PASSWORD "{sql_password}"')
    
    # 3. Set to current process environment (Immediate use)
    os.environ["MSSQL_SA_PASSWORD"] = sql_password

    # 4. Clean up existing container if it exists
    subprocess.run(f"docker rm -f {container_name}", shell=True, capture_output=True)

    print(f"MSSQL_SA_PASSWORD is set in the environment. Deploying SQL Server container with this password... {sql_password}")

    # 5. Run the container
    # We pull the password directly from the environment we just set
    cmd = [
        "docker", "run",
        "-e", "ACCEPT_EULA=Y",
        "-e", f"MSSQL_SA_PASSWORD={os.environ['MSSQL_SA_PASSWORD']}",
        "-p", "1433:1433",
        "--name", container_name,
        "--restart", "unless-stopped",
        "-d", "mcr.microsoft.com/mssql/server:2022-latest"
    ]

    print("🚀 Starting SQL Server container...")
    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode == 0:
        print("⏳ Waiting 15 seconds for SQL Engine to initialize...")
        time.sleep(15)
        
        # 6. Verify if it actually stayed running
        check = subprocess.run(f"docker inspect -f '{{{{.State.Status}}}}' {container_name}", 
                               shell=True, capture_output=True, text=True)
        status = check.stdout.strip()
        
        if status == "running":
            print(f"✅ SUCCESS! SQL Server is running.")
            print(f"📌 Your password is saved in the Windows Environment Variable: MSSQL_SA_PASSWORD")
        else:
            print(f"❌ CRASHED: Status is '{status}'. Checking logs...")
            logs = subprocess.run(f"docker logs {container_name}", shell=True, capture_output=True, text=True)
            print(f"--- LOG TAIL ---\n{logs.stdout[-500:]}")
    else:
        print(f"❌ Docker Run Failed: {result.stderr}")        

def install_dev_stack():
    # 1. Standard Winget tools (removed Maven from here since we do it manually now)
    tools = {
        "Visual Studio 2022": "Microsoft.VisualStudio.2022.Community",
        "Visual Studio Code": "Microsoft.VisualStudioCode",
        "Git": "Git.Git",
        "Microsoft OpenJDK 21": "Microsoft.OpenJDK.21",
        "C# .NET 8 SDK": "Microsoft.DotNet.SDK.8",
        "CMake": "Kitware.CMake"
    }

    for name, tool_id in tools.items():
        install_or_update_app(name, tool_id)

    # 2. Manual Maven Install
    install_maven_manual()

    # 3. vcpkg Setup
    setup_vcpkg()
    
    # 4. Visual Studio Workload Sync
    if os.path.exists(VS_CONFIG_PATH):
        print("\n--- 🏗️  Syncing Visual Studio Workloads ---")
        vs_installer = r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe"
        if os.path.exists(vs_installer):
            # Using raw string (r"") to prevent \P escape errors
            vs_path = r"C:\Program Files\Microsoft Visual Studio\2022\Community"
            run_command(
                f'"{vs_installer}" modify --installPath "{vs_path}" --config "{VS_CONFIG_PATH}" --passive --norestart',
                "Applying .vsconfig to Visual Studio"
            )

    # 5. Deploy SQL Server Container
    deploy_sql_server()

    print("\n" + "="*40)
    print("✅ FULL STACK SETUP COMPLETE!")
    print("="*40)
    print("1. RESTART your terminal (Crucial to refresh PATH/JAVA_HOME).")
    print("2. Run 'vcpkg integrate install' if you use C++.")
    print("="*40)

if __name__ == "__main__":
    if os.name != 'nt':
        print("❌ This script is designed for Windows.")
        sys.exit(1)
        
    import ctypes
    if not ctypes.windll.shell32.IsUserAnAdmin():
        print("⚠️  Warning: This script should be run as ADMINISTRATOR for best results.")
    
    install_dev_stack()