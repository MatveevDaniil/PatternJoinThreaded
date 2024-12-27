# PatternJoinThreaded
This project is an experimental version of PatternJoin project, and is designed to test parallelization capabilities of our algorithms. In order to read more about the main algorithms please visit our [C++](https://github.com/MatveevDaniil/PatternJoin) or [R](https://matveevdaniil.r-universe.dev/RPatternJoin) packages.

## What is Edit Similarity Join?
[_Edit distance_](https://en.wikipedia.org/w/index.php?search=Edit+distance&title=Special:Search&ns0=1) between two words is the minimal number of _elementary operations_ needed to transform one word to another. In this project, we consider [_Levenshtein distance_](https://en.wikipedia.org/wiki/Levenshtein_distance) (allows _substitutions_ and _insertions_/_deletions_ of single letters) and [_Hamming distance_](https://en.wikipedia.org/wiki/Hamming_distance) (allows _substitutions_ of any letters and _insertions_/_deletions_ of letters in the end). 

_Edit Similarity Join_ is a specific problem when we have a set of words $X=\\{w_1, w_2, \ldots, w_n\\}$ and we want to find all pairs of 'similar words' $Y_c=\\{(w_i, w_j): distance(w_i, w_j) < c\\}$, where $c \in \mathbb{N}_0$ is called a _cutoff_ parameter.

## Motivation
Let's briefly introduce the problem why this project was originally developed.

### Intro to TCR(BCR) profiles 
__T-cells__ and __B-cells__ are types of white blood immune cells which play a major role in human immunity. On the surface of each T/B-cell we can find receptors (__TCR__/__BCR__) - special molecules that are responsible for recognizing antigens. The collection of all TCRs/BCRs is called __TCR (BCR) profile__. One way to represent an organism's TCR/BCR profile is to collect the genetic sequences of most variable region (CDR3) of each TCR/BCR. Recent research (for instance [[1]](https://www.nature.com/articles/s41467-019-09278-8), [[2]](https://academic.oup.com/bioinformatics/article/36/6/1731/5686386)) shows that it is useful to (1) represent these profiles as graphs with nodes being TCR/BCR and edge weights edit distances between nodes and (2) run some network analysis on these graphs (take a look at the [NAIR](https://github.com/mlizhangx/Network-Analysis-for-Repertoire-Sequencing-) project if you are interested in network analysis in immunology). In order to build these graphs we need efficient edit similarity join software and this is the main purpose of this project.


## Parallel Hash Set and Hash Map Performance benchmarks
To run performance benchmarks you need to compile performance_testing subproject:

```shell
> cd performance_testing
> mkdir build; cd build
> cmake -DCMAKE_BUILD_TYPE=Release ..; make
> cd ..
```

Then in order you can run hash_set and hash_map tests using
```shell
# assuming you are in PatternJoinThreaded/performance_testing directory
> ./build/threaded_hash_sets
> ./build/threaded_hash_maps
```

When the tests will finish generate figures using
```shell
> pip install pandas seaborn matplotlib
> python plot_results.py
```

## Parallel Partition-Pattern Join Algorithm
To run partition_pattern algorithm using several threads firstly compile the project
```shell
# assuming you are in PatternJoinThreaded directory
> mkdir build; cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make; cd ..
```

Below we give an example of algorithm launch:
```shell
> export FILE_NAME=./performance_testing/test_data/P00245-aa
> export CUTOFF=2
> export METRIC_TYPE=L
> export METHOD=partition_pattern
> export INCLUDE_DUPLICATES=false
> ./build/pattern_join --file_name $FILE_NAME --cutoff $CUTOFF --metric_type $METRIC_TYPE --method $METHOD --include_duplicates $INCLUDE_DUPLICATES
```

### Full list of arguments:
- `<file_name>`: The path to the input file.
- `<cutoff>`: The edit distance cutoff (`0`, `1` or `2`). If `cutoff` = 0, then the value of `metric_type`, `method`, and `include_duplicates` does not matter.
- `<metric_type>`: The edit distance metric (`L` for Levenshtein, `H` for Hamming).
- `<method>`: For threaded implementation we currently tested only `partition_pattern`. 
- `<include_duplicates>`: Consider duplicates in input (`true` or `false`). If `false` the program will ignore duplicate strings in the input and output unique pairs of strings. If `true`, the program will treat duplicate strings in the input as a pair (index, string) and output pairs of indices. 

### Input file format
List of words separated by `\n`: `<word_1>\n<word_2>\n...`.

### Output file format
If `include_duplicates = true`: Space-separated pairs of words separated by `\n`: `<word_i> <word_j>\n...`.
If `include_duplicates = false`: Space-separated pairs of indeces separated by `\n`: `<idx_i> <idx_j>\n...`.