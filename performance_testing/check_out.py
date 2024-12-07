import Levenshtein

fname = 'build/check_out'
i = 0
with open(fname, 'r') as file:
  while True:
    line = file.readline()
    if not line:
      break
    a, b = line.split()
    if Levenshtein.distance(a, b) > 3:
      print(a, b, Levenshtein.distance(a, b))
      break
    i += 1
    if i % 10000 == 0:
      print(i)