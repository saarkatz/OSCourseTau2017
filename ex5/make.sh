# This script is for making the module without needing to move it
# to a directory which is not shared with host on virtual machine.

echo "Making kernel module"

# Save original pwd
lastPWD=$PWD

# Create temporary dir name
myDir="/tmp/MyDir$(date +"%H-%M-%S")"

# Create temporary dir
mkdir $myDir

# Copy content to temporary dir
cp $lastPWD/*.c $myDir
cp $lastPWD/*.h $myDir
cp $lastPWD/Makefile $myDir

# cd temporary directory
cd $myDir

# Invoke make
echo "Starting make..."
make
echo "make finished"

# Copy *.ko back
cp $myDir/*.ko $lastPWD

# Return to working directory
cd $lastPWD

# Remove temporary dir along with all its content
rm -r $myDir

echo "Finished"