#cmake -G "Unix Makefiles" .
cmake -S . -B build
cmake --build build -j --clean-first
cmake --install build
QF="T-q1e4-W"
W=(1 10 50 90 99)
for w in ${W[@]}; do
    mkdir -p Experiments/$QF$w
    ./Index $PWD $QF$w 128 128
done
# lldb -- Index $PWD $QF $1 $2
# params=(128 256 512 1024 2048)
# for p in "${params[@]}"; do
#     ./Index $PWD $QF "$p" "$p"
# done
#python visualizeHopTree.py
