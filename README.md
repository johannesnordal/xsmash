# Requirements

+ [GCC](https://gcc.gnu.org/) >= 4.8.5
+ [Zlib](https://zlib.net/)

# Installation

```
git clone --recurse-submodules https://github.com/johannesnordal/xsmash.git
cd xsmash
make
```

# Usage

Assuming we are in the `xsmash` directory, running

```
bin/xsketch
```

displays the command line interface:

```
Usage: bin/xsketch [options] <in.fastq>

Options:
  -k    Size of kmers [default: 21]
  -c    Candidate set limit [default: 1]
```
