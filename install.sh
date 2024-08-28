sudo apt update -y
sudo apt install libsdl2-dev  libsndfile1-dev libboost-all-dev -y

cd ws-app
mkdir build
cd build
cmake ..
make