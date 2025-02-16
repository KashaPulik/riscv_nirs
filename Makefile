riscv:
	riscv64-unknown-linux-gnu-g++ -O0 -mcpu=spacemit-x60 -march=rv64gc_zba_zbb_zbc_zbs main.cpp -o prog.gcc

x86-64:
	g++ -Wall -O0 -o prog main.cpp

clean:
	rm -rf prog prog.gcc