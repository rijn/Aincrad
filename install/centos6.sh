sudo yum install -y svn texinfo-tex flex zip libgcc.i686 glibc-devel.i686 gcc gcc-c++ cmake
sudo yum remove -y clang

mkdir ~/sourceInstallations
cd ~/sourceInstallations
svn co https://gcc.gnu.org/svn/gcc/tags/gcc_5_1_0_release/

svn co http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_380/final llvm_RELEASE_380
cd llvm_RELEASE_380/tools
svn co http://llvm.org/svn/llvm-project/cfe/tags/RELEASE_380/final clang
cd ../projects
svn co http://llvm.org/svn/llvm-project/compiler-rt/tags/RELEASE_380/final compiler-rt
svn co http://llvm.org/svn/llvm-project/libcxx/tags/RELEASE_380/final libcxx
svn co http://llvm.org/svn/llvm-project/libcxxabi/tags/RELEASE_380/final libcxxabi

cd ~/sourceInstallations
wget https://www.python.org/ftp/python/2.7.9/Python-2.7.9.tgz
tar -xvf Python-2.7.9.tgz

SWAP=/tmp/swap
dd if=/dev/zero of=$SWAP bs=1M count=500
mkswap $SWAP
sudo swapon $SWAP

cd ~/sourceInstallations/gcc_5_1_0_release/
./contrib/download_prerequisites
cd ..
mkdir gcc_5_1_0_release_build/
cd gcc_5_1_0_release_build/
../gcc_5_1_0_release/configure && make && sudo make install && echo "success"

cd ~/sourceInstallations/Python-2.7.9
./configure && make && sudo make install

mkdir ~/sourceInstallations/llvm_RELEASE_380_build
cd ~/sourceInstallations/llvm_RELEASE_380_build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/local/bin/gcc -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ ../llvm_RELEASE_380 && make && sudo make install && echo success

echo "/usr/local/lib64" > usrLocalLib64.conf
sudo mv usrLocalLib64.conf /etc/ld.so.conf.d/
echo "/usr/local/lib" > usrLocalLib.conf
sudo mv usrLocalLib.conf /etc/ld.so.conf.d/
sudo ldconfig

sudo swapoff $SWAP
rm /tmp/swap

cd ~
rm -rf sourceInstallations
