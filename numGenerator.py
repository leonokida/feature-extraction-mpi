import random
import sys

f = open('randomNums.txt', 'w')

rng = 100 if len(sys.argv) == 1 else int(sys.argv[1])

for i in range(rng):
    print(random.uniform(0, 100), file=f)

f.close()