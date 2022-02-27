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
bin/sketch
```

displays the command line interface:

```
Usage: bin/sketch [options] <in.fasta | in.fastq>

Options:
  -k    Size of kmers [default: 21].
  -c    Candidate set limit [default: 1].
  -s    Size of min hash [default: 1000]. Ignored with -x and -X options.
  -x    Include all hashes that have a value lower than 9999999776999205UL.
  -X    Include all hashes that have a value lower than X.
```
