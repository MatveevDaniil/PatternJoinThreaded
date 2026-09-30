from concurrent.futures import ThreadPoolExecutor
import itertools
import random

import pytest
from patternjoin import join


def distance(a, b, metric):
  if metric == 'H':
    return abs(len(a)-len(b)) + sum(x != y for x, y in zip(a, b))
  row = list(range(len(b)+1))
  for i, x in enumerate(a):
    nxt = [i+1]
    for j, y in enumerate(b):
      nxt.append(min(nxt[-1]+1, row[j+1]+1, row[j]+(x != y)))
    row = nxt
  return row[-1]


def oracle(a, b, cutoff, metric):
  return [(i, j) for i, x in enumerate(a) for j, y in enumerate(b)
          if distance(x, y, metric) <= cutoff]


@pytest.mark.parametrize('threads', [1, 2, 4])
@pytest.mark.parametrize('metric', ['L', 'H'])
@pytest.mark.parametrize('cutoff', [0, 1, 2])
def test_oracle(threads, metric, cutoff):
  words = [''.join(p) for n in range(5)
           for p in itertools.product('ab', repeat=n)]
  a = words + ['a', '*', 'a\0b', 'a b']
  b = list(reversed(words)) + ['', '\0', '_']
  expected = oracle(a, b, cutoff, metric)
  assert join(a, b, cutoff=cutoff, metric=metric,
              threads=threads) == expected
  assert join(b, a, cutoff=cutoff, metric=metric,
              threads=threads) == sorted((j, i) for i, j in expected)


@pytest.fixture(scope='module')
def parallel_case():
  a = ['cat' + str(i % 80) for i in range(130)]
  b = ['bat' + str(i % 80) for i in range(210)]
  return a, b, oracle(a, b, 2, 'L')


@pytest.mark.parametrize('threads', [1, 2, 4, 8])
def test_parallel_duplicates(parallel_case, threads):
  a, b, expected = parallel_case
  for _ in range(5):
    assert join(a, b, cutoff=2, threads=threads) == expected


def test_simultaneous_calls(parallel_case):
  a, b, expected = parallel_case
  with ThreadPoolExecutor(max_workers=4) as pool:
    futures = [pool.submit(join, a, b, cutoff=2, threads=n)
               for n in [1, 2, 4, 2]]
    assert all(f.result() == expected for f in futures)


@pytest.mark.parametrize('cutoff', [0, 1, 2])
def test_self_join_and_generators(cutoff):
  a = ['', 'cat', 'cat', 'bat']
  expected = oracle(a, a, cutoff, 'L')
  assert join(iter(a), cutoff=cutoff, threads=2) == expected
  assert join(a, a, cutoff=cutoff, threads=1) == expected
  assert join([], a, cutoff=cutoff) == []
  assert join(a, [], cutoff=cutoff) == []
  assert join([], cutoff=cutoff) == []


def test_long_random_strings():
  rng = random.Random(42)
  a = [''.join(rng.choices('abc', k=80)) for _ in range(12)]
  b = [s[:-1] + 'a' for s in a] + [s[1:] for s in a]
  for metric in ('L', 'H'):
    for cutoff in (1, 2):
      assert join(a, b, cutoff=cutoff, metric=metric, threads=4) == \
        oracle(a, b, cutoff, metric)


@pytest.mark.parametrize('values', ['abc', b'abc', [1], [None]])
def test_invalid_strings(values):
  with pytest.raises(TypeError):
    join(values)
  with pytest.raises(TypeError):
    join(['a'], values)


def test_unicode_rejected():
  with pytest.raises(ValueError):
    join(['é'])
  with pytest.raises(ValueError):
    join(['a'], ['😀'])


@pytest.mark.parametrize('option,value', [
  ('cutoff', -1), ('cutoff', 3), ('cutoff', True),
  ('cutoff', 1.5), ('cutoff', '1'), ('cutoff', 10**100),
  ('metric', 'X'), ('metric', ''), ('metric', None),
  ('threads', -1), ('threads', True), ('threads', 1.5),
  ('threads', '2'), ('threads', 10**100)])
def test_invalid_options(option, value):
  with pytest.raises((TypeError, ValueError, OverflowError)):
    join(['a'], **{option: value})


def test_openmp_default(tmp_path):
  import os
  import subprocess
  import sys
  env = dict(os.environ, OMP_NUM_THREADS='2')
  subprocess.run([sys.executable, '-c',
    'from patternjoin import join; '
    'assert join(["a"], ["b"]) == [(0, 0)]'],
    check=True, env=env, cwd=tmp_path)
