# Canonical Huffman Compression
An implementation of Canonical Huffman Encoder/Decoder in C, compatible for running in Pintos kernel.

## Usage

In Linux

```
$ make
$ ./huffc -c ./tests/sample1.txt
$ ./huffc -d ./tests/sample1.cmp

// To run tests
$ ./testhuffc ./tests/sample1.txt
```

In Pintos, it needs project 3 virtual memory to run.

```
$ cd src/vm/build
$ pintos -v -k --smp 2 --kvm  --filesys-size=8              \
    -p ../../examples/testhuffc -a testhuffc                \
    -p ../../examples/huffc -a huffc                        \
    -p ../../tests/vm/sample.txt -a sample.txt              \
    --swap-size=8 -- -q -f run 'testhuffc sample.txt'
```
