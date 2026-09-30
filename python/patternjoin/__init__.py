"""String similarity joins. Returned indices are zero-based."""

from ._core import join as _join

__all__ = ["join"]


def _strings(values, name):
  if isinstance(values, (str, bytes)):
    raise TypeError(f"{name} must be an iterable of strings")
  result = list(values)
  for value in result:
    if not isinstance(value, str):
      raise TypeError(f"{name} must contain only strings")
    if not value.isascii():
      raise ValueError("only ASCII strings are supported")
  return result


def join(a, b=None, *, cutoff=1, metric="L", threads=0):
  """Return sorted (i, j) pairs whose string distance is <= cutoff.

  Omitting b means join(a, a), including the diagonal and both
  orientations. Duplicates keep their original indices. Empty
  inputs and empty strings are supported. Strings must be ASCII.
  metric is L (Levenshtein) or H (positional mismatches plus length
  difference). cutoff is 0, 1 or 2. threads=0 uses OpenMP's default;
  positive values select the thread count for this call. Cutoff 0
  uses serial exact matching. Results are returned, never written.
  """
  for name, value in (("cutoff", cutoff), ("threads", threads)):
    if isinstance(value, bool) or not isinstance(value, int):
      raise TypeError(f"{name} must be an integer")
  if not isinstance(metric, str) or metric not in ("L", "H"):
    raise ValueError("metric must be L or H")
  left = _strings(a, "a")
  right = left if b is None else _strings(b, "b")
  return _join(left, right, cutoff, metric, threads)
