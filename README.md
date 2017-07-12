# selectION
Rapid linking of long reads to a reference genome

### Dependencies
Selection is written in C++11 and requires the following libraries:

* boost filesystem
* boost program_options
* boost system
* libhdf5

Boost is available through standard package sources. Libhdf5 is downloaded and build by the install script. The index building uses SSE3 acceleration.

### Installation
#### Linux
In order to download, build and install selectION, execute the following commands:

    git clone https://github.com/PayGiesselmann/selectION
    cd selectION
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    make install

If you wish to install the software in any other than the default directory, use the following cmake command:

    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<install_prefix> ..

If everything's gone right, typing 'selection' into the command line should give you some output like this:

    Program:    SelectION
    Usage:      selection <command> [options]
    Commands:   index : Build FM-Index for reference sequence
                scan : Scan input for reads matching specified positions

#### Others
Not documented yet. SelectION is cross-platform software and will, the dependencies resolved, build on any x86/x64 system.

### Usage

#### Index
Before you can use selectION, you have to build an index for the reference genome. Input can be any FASTA file containing one or multiple sequences. Multiple input files are not supported yet.

    selection index -t 8 ref.fa

This will build the index using eight threads and create a file _ref.fa.h5_ in the same directory. Additional options are available, for human genome applications the defaults should however work fine.

#### Scan
Estimate positions for all reads in _input.fq_ and write results to _out.sam_ in current directory. Note that lines will be appended to existing output files.

    selection scan -t 8 ref.fa input.fq ./ --sam ./out.sam

Estimate position for all reads in _input.fq_ and write reads matching region of interest in _roi.txt_ to current directory

    selection scan -t 8 ref.fa input.fq ./ --filter ./roi.txt

The syntax for the roi.txt is as follows. You can specifiy as many selectors as you want.

    # chromosome
    X
    # single spot
    X;67542032
    # region of interest
    X;67542032;67732619
    # with custom name (default: X_146993569_146993569)
    X;146993569;146993569;FMR1

For either a complete chromosome, a specific spot or a region defined by start and stop. Last column may contain a custom name to use for the output files. The naming of the chromosome must match the spelling in the reference e.g. _chrX_ is not equal to _X_!

Support for input _fast5_ files is coming soon, for the moment we recommend using poretools to extract basecalled sequences from ONT _fast5_ files.
