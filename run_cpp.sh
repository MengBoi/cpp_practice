f=${1:-main.cpp}
out=${f%.cpp}
case $out in /*) ;; *) out=./$out ;; esac
clang++ -std=c++23 -Wall "$f" -o "$out" && "$out"
