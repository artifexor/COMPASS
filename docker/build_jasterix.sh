mkdir -p /app/workspace/jasterix/ub14_build
cd /app/workspace/jasterix/ub14_build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j16
sudo make install

cd /app/workspace/compass/docker
