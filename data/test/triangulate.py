infile = "/home/rfan/code/OpenGL/facialExps/data/test/1.obj"
outfile = "/home/rfan/code/OpenGL/facialExps/data/test/1_t.obj"

with open(infile, 'r') as file:
    with open(outfile, 'w') as output:
        for line in file:
            if not line.startswith('f'):
                output.write(line)
            else:
                ls = line.split()
                if len(ls) < 5:
                    output.write(line)
                else:
                    ls1 = [s for s in ls[:4]]
                    output.write(" ".join(ls1) + "\n")
                    ls2 = [s for s in ls[:2]] + [s for s in ls[3:5]]
                    output.write(" ".join(ls2) + "\n")
