#rm -rf Experiments/*
QF="qGrow90mil"
#QF="knnTestQueries.txt"
mkdir -p Experiments/$QF
#cmake -G "Unix Makefiles" .
cmake -S . -B build
cmake --build build -j --clean-first
cmake --install build
# lldb -- Index $PWD $QF $1 $2
./Index $PWD $QF $1 $2
# params=(16 32 64 128 256 512)
# for p in "${params[@]}"; do
#     ./Index $PWD $QF "$p" 512
# done
#python visualizeHopTree.py
