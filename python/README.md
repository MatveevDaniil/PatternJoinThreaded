# PatternJoin for Python

```python
from patternjoin import join

join(["cat", "dog"], ["bat", "cat"], cutoff=1, threads=4)
# [(0, 0), (0, 1)]
```

`join(a, b=None, *, cutoff=1, metric="L", threads=0)` returns sorted
zero-based index pairs. `join(a)` means `join(a, a)`: it includes
the diagonal and both orientations. Duplicate strings retain their
original positions. Empty iterables and empty strings are valid.
Inputs can be iterables of ASCII strings, including generators.
Non-ASCII strings are rejected because the C++ routines count bytes.

Cutoffs are 0, 1 or 2. `metric="L"` is Levenshtein distance;
`metric="H"` counts positional mismatches plus the length difference.
Nonzero cutoffs use the existing semi-pattern implementation with
the smaller input indexed. Cutoff 0 uses serial exact matching.

`threads=0` uses the OpenMP default, configurable with
`OMP_NUM_THREADS` before starting Python. Use `threads=1` for serial
execution or a positive number to set the count for one call.
Calls release the GIL. More threads need not make small joins faster.
The package returns results in memory and does not create files.

## Build and test

A C++20 compiler, CMake, OpenMP and TBB are required. On macOS:

```sh
brew install libomp tbb
python -m pip install .
python -m pip install pytest
python -m pytest python/tests
```

On Linux, install the compiler's OpenMP runtime and TBB development
package first. If CMake cannot locate TBB, set `CMAKE_PREFIX_PATH`
to its installation prefix. Native Windows builds are not tested.

Build distribution archives with `python -m build`. Locally built
wheels depend on the installed OpenMP/TBB libraries. For a portable
macOS wheel, run `delocate-wheel -w wheelhouse dist/*.whl` and test
the repaired wheel in a fresh environment. Linux wheels need the
corresponding `auditwheel repair` workflow. Nothing is published
to PyPI by these commands.

## Releases

`.github/workflows/publish.yml` builds and tests CPython 3.9–3.14
wheels for Linux x86-64 and macOS 15+ (Intel and Apple Silicon),
bundling the native runtime libraries. Pull requests only build/test.

Configure a PyPI Trusted Publisher for project `patternjoin`, owner
`MatveevDaniil`, repository `PatternJoinThreaded`, workflow
`publish.yml`, environment `pypi`. No API token is needed.
For a trial upload, configure the same publisher on TestPyPI with
environment `testpypi`, then manually run the workflow on `main`
with `testpypi` enabled.

To publish, update the version in `pyproject.toml`, merge the change,
and publish a GitHub release with the matching tag (e.g. `v0.1.0`).
After all wheel tests pass, the workflow uploads wheels and the
source archive to PyPI. Each release needs a new version.
