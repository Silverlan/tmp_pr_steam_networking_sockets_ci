import os
import subprocess
import sys
from sys import platform
from pathlib import Path

# ninja
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

# Based on build instructions: https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md
gns_root = deps_dir +"/GameNetworkingSockets"
os.chdir(gns_root)
mkdir("GameNetworkingSockets",cd=True)

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
    os.system("\"" +os.path.normpath(vsdevcmd_path) + "\"&" +'cmake -S . -B build -G Ninja')

    os.chdir("build")
    os.system("\"" +os.path.normpath(vsdevcmd_path) + "\"&ninja")
else:
    print_msg("Installing required GameNetworkingSockets packages...")
    commands = [
        "apt install libssl-dev",
        "apt install libprotobuf-dev protobuf-compiler"
    ]
    install_system_packages(commands)

    print_msg("Building GameNetworkingSockets...")
    mkdir("build",cd=True)
    subprocess.run(["cmake","-G","Ninja",".."],check=True)
    subprocess.run(["ninja"],check=True)

cmake_args.append("-DDEPENDENCY_VALVE_GAMENETWORKINGSOCKETS_INCLUDE=" +gns_root +"/include")
cmake_args.append("-DDEPENDENCY_VALVE_GAMENETWORKINGSOCKETS_LIBRARY=" +gns_root +"/build/src/GameNetworkingSockets.lib")
cmake_args.append("-DDEPENDENCY_GAMENETWORKINGSOCKETS_BINARY_DIR=" +gns_root +"/build/bin/")
