import os
import subprocess
import sys
import stat
from sys import platform
from pathlib import Path

def print_all_files(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            print(os.path.join(root, file))

# ninja
os.chdir(deps_dir)
ninja_root = deps_dir +"/ninja"
if platform == "linux":
    ninja_executable_name = "ninja"
else:
    ninja_executable_name = "ninja.exe"
if not Path(ninja_root).is_dir():
    mkdir("ninja",cd=True)
    print_msg("Downloading ninja...")
    os.chdir(ninja_root)
    if platform == "linux":
        http_extract("https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip")
    else:
        http_extract("https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip")
        st = os.stat('ninja')
        os.chmod('ninja', st.st_mode | stat.S_IEXEC)
    print_all_files(ninja_root)

# Based on build instructions: https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md
os.chdir(deps_dir)
gns_root = deps_dir +"/GameNetworkingSockets"
if not Path(gns_root).is_dir():
    print_msg("GameNetworkingSockets not found, downloading...")
    git_clone("https://github.com/ValveSoftware/GameNetworkingSockets.git")

os.chdir(gns_root)
reset_to_commit("505c697d0abef5da2ff3be35aa4ea3687597c3e9")

script_path = os.path.abspath(__file__)
print_msg("Applying patch...")
subprocess.run(["git","apply",os.path.dirname(script_path) +"/GameNetworkingSockets.patch"],check=False,shell=True)

cp(ninja_root +"/" +ninja_executable_name,gns_root +"/")
ninja_path_to_executable = gns_root +"/"
sys.path.insert(0,ninja_path_to_executable)
os.environ['PATH'] += os.pathsep + ninja_path_to_executable

if platform == "win32":
    # Note: Using the vcpkg in deps directory will NOT work!
    gns_vcpkg_root = gns_root +"/vcpkg"
    if not Path(gns_vcpkg_root).is_dir():
        print_msg("vcpkg not found, downloading...")
        git_clone("https://github.com/Microsoft/vcpkg.git")

    os.chdir(gns_vcpkg_root)
    reset_to_commit("3b7578831da081ba164be30da8d9382a64841059")
    os.chdir("..")
    if platform == "linux":
        subprocess.run([gns_vcpkg_root +"/bootstrap-vcpkg.sh","-disableMetrics"],check=True,shell=True)
    else:
        subprocess.run([gns_vcpkg_root +"/bootstrap-vcpkg.bat","-disableMetrics"],check=True,shell=True)

    subprocess.run([gns_vcpkg_root +"/vcpkg","install","--triplet=x64-windows"],check=True,shell=True)

    print_msg("Building GameNetworkingSockets...")
    vsdevcmd_path = determine_vsdevcmd_path(deps_dir)
    os.system("\"" +os.path.normpath(vsdevcmd_path) + "\" -arch=x64&" +'cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo')

    os.chdir("build")
    os.system("\"" +os.path.normpath(vsdevcmd_path) + "\" -arch=x64&ninja")
else:
    print_msg("Installing required GameNetworkingSockets packages...")
    commands = [
        "apt install libssl-dev",
        "apt install libprotobuf-dev protobuf-compiler"
    ]
    install_system_packages(commands, no_confirm)

    print_msg("Building GameNetworkingSockets...")
    mkdir("build",cd=True)
    subprocess.run(["cmake","-G","Ninja",".."],check=True)
    subprocess.run(["ninja"],check=True)

cmake_args.append("-DDEPENDENCY_VALVE_GAMENETWORKINGSOCKETS_INCLUDE=" +gns_root +"/include")
cmake_args.append("-DDEPENDENCY_VALVE_GAMENETWORKINGSOCKETS_LIBRARY=" +gns_root +"/build/src/GameNetworkingSockets.lib")
cmake_args.append("-DDEPENDENCY_GAMENETWORKINGSOCKETS_BINARY_DIR=" +gns_root +"/build/bin")
