#rm -rf Experiments/*
QF="q1e4-25"
#QF="knnTestQueries.txt"
mkdir -p Experiments/$QF
#cmake -G "Unix Makefiles" .
cmake -S . -B build
cmake --build build -j --clean-first
cmake --install build
# lldb -- Index $PWD $QF $1 $2
./Index $PWD $QF $1 $2
# params=(128 256 512 1024 2048)
# for p in "${params[@]}"; do
#     ./Index $PWD $QF "$p" "$p"
# done
#python visualizeHopTree.py
